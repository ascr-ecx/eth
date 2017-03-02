#include <mpi.h>
#include <unistd.h>
#include <vector>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>

#include "MyServerSocket.h"

#include <vtkDataSet.h>
#include <vtkDataArraySelection.h>
#include <vtkImageData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLPolyDataReader.h>

#include <vtkDataSetWriter.h>

#define VTI 1
#define VTU 2
#define VTP 3

using namespace std;

int  mpir, mpis;

void
syntax(char *a)
{
    if (mpir == 0)
    {
      cerr << "syntax: " << a << " template layoutfile [variable] [options]\n";
      cerr << "options:\n";
      cerr << "  -m r s             set rank, size (for testing)\n";
      cerr << "  -t n               n timesteps (default 1)\n";
      cerr << "  -R n               Repeat factor (default 1)\n";
    }
    MPI_Finalize();
    exit(1);
}

template<class T, class TReader> vtkDataSet *ReadAnXMLFile(const char*fileName, char *var)
{
	TReader *reader = TReader::New();
  reader->SetFileName(fileName);
	if (var)
	{
		reader->UpdateInformation();
		vtkDataArraySelection *sel = reader->GetPointDataArraySelection();
		for (int i = 0; i < sel->GetNumberOfArrays(); i++)
			if (strcmp(sel->GetArrayName(i), var))
				sel->DisableArray(sel->GetArrayName(i));
			else
				sel->EnableArray(sel->GetArrayName(i));
	}
  reader->Update();
	T *t = T::New();
	t->ShallowCopy(reader->GetOutput());
	reader->Delete();
  return vtkDataSet::SafeDownCast(t);
}

void
GetPort(char *layout, int r, int &port)
{
	int lerr = 0, gerr;

  string server;
  ifstream ifs(layout);
  for (int i = 0; !lerr && i <= mpir; i++)
  {
    ifs >> server >> port;
    if (ifs.eof())
      lerr = 1;
  }

  MPI_Allreduce(&lerr, &gerr, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  if (gerr)
  {
    if (r == 0)
      cerr << "not enough vis servers\n";
    MPI_Finalize();
    exit(1);
  }
}

vtkSocket *
OpenClientSocket(MyServerSocket *serverSocket, int rank)
{
	vtkSocket *skt = NULL;
;
	int vis_ready, ok;
	if (mpir == 0)
	{
		do
		{
			if (serverSocket->ConnectionWaiting())
				skt = (vtkSocket *)serverSocket->WaitForConnection(1);
		}
		while (skt == NULL);

		vis_ready = skt == NULL ? 0 : 1;
		ok = vis_ready;
	}

	MPI_Bcast(&vis_ready, 1, MPI_INT, 0, MPI_COMM_WORLD);

	if (vis_ready)
	{
		if (mpir > 0)
		{
			skt = (vtkSocket *)serverSocket->WaitForConnection(10000);
			ok = skt == NULL ? 0 : 1;
		}
	}

	int global_ok;
	MPI_Allreduce(&ok, &global_ok, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

	if (vis_ready && !global_ok)
	{
		if (mpir == 0)
			std::cerr << "Root node saw vis server, but the others could not connect\n";

    MPI_Finalize();
    exit(1);
	}

  return skt;
}

int
main(int argc, char *argv[])
{
  int   	nt = 1;
	int 		nr = 1;
  char  	*layoutfile = NULL;
  char  	*tmplate = NULL;
  char  	*var = NULL;
	int   	datatype;

	vector<string> variables;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &mpis);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpir);

	tmplate = argv[1];
	layoutfile = argv[2];

  for (int i = 3; i < argc; i++)
    if (argv[i][0] == '-') 
      switch(argv[i][1])
      {
        case 'm': mpir = atoi(argv[++i]); mpis = atoi(argv[++i]); break;
        case 't': nt = atoi(argv[++i]); break;
        case 'r': nr = atoi(argv[++i]); break;
        default: 
          syntax(argv[0]);
      }
    else if (var == NULL)
			var = argv[i];
		else
      syntax(argv[0]);

  if (! layoutfile || ! tmplate)
    syntax(argv[0]);

	if (! strcmp(tmplate + strlen(tmplate) -3, "vti"))
		datatype = VTI;
	else if (! strcmp(tmplate + strlen(tmplate) -3, "vtu"))
		datatype = VTU;
	else if (! strcmp(tmplate + strlen(tmplate) -3, "vtp"))
		datatype = VTP;
	else
	{
		if (mpir == 0)
			cerr << "Only .vtu and .vti files supported\n";
    MPI_Finalize();
    exit(1);
	}

	vector<vtkDataSet *> timesteps;

	for (int t = 0; t < nt; t++)
	{
		char filename[1024];
		sprintf(filename, tmplate, t, mpir);
		if (datatype == VTI)
			timesteps.push_back(ReadAnXMLFile<vtkImageData, vtkXMLImageDataReader>(filename, var));
		else if (datatype == VTP)
			timesteps.push_back(ReadAnXMLFile<vtkPolyData, vtkXMLPolyDataReader>(filename, var));
		else
			timesteps.push_back(ReadAnXMLFile<vtkUnstructuredGrid, vtkXMLUnstructuredGridReader>(filename, var));
	}

	MPI_Barrier(MPI_COMM_WORLD);

	int port;
  GetPort(layoutfile, mpir, port);

	MyServerSocket *serverSocket = MyServerSocket::New();
	serverSocket->CreateServer(port);

	vtkDataSetWriter *wr = vtkDataSetWriter::New();
	wr->WriteToOutputStringOn();

	vtkSocket *skt = OpenClientSocket(serverSocket, mpir);

  serverSocket->CloseSocket();
  serverSocket->Delete();

	for (int r = 0; r < nr; r++)
	{
		for (int t = 0; t < nt; t++)
		{
			if (mpir == 0)
			{
				int flag = 1;
				skt->Send((const void *)&flag, sizeof(flag));
			}

			// If more than writing the raw data is required, a VTK pipeline 
			// would be inserted here

			wr->SetInputData(timesteps[t]);
			wr->Update();
    
			int sz = wr->GetOutputStringLength();
			void *ptr = wr->GetOutputString();

			skt->Send((const void *)&sz, sizeof(sz));
			skt->Send((const void *)ptr, sz);
		}
	}

  if (mpir == 0)
  {
    int flag = 0;
    skt->Send((const void *)&flag, sizeof(flag));
  }

	wr->Delete();

	skt->CloseSocket();
	skt->Delete();

  MPI_Finalize();
}
