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
#include <vtkImageData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLUnstructuredGridReader.h>

#include <vtkDataSetWriter.h>

#define VTI 1
#define VTU 2

using namespace std;

int  mpir, mpis;

void
syntax(char *a)
{
    if (mpir == 0)
    {
      cerr << "syntax: " << a << " template layoutfile [options]\n";
      cerr << "options:\n";
      cerr << "  -m r s             set rank, size (for testing)\n";
      cerr << "  -t n               n timesteps (default 1)\n";
    }
    MPI_Finalize();
    exit(1);
}

template<class TReader> vtkDataSet *ReadAnXMLFile(const char*fileName)
{
   TReader *reader = TReader::New();
   reader->SetFileName(fileName);
   reader->Update();
   // reader->GetOutput()->Register(reader);
   return vtkDataSet::SafeDownCast(reader->GetOutput());
}

void
GetPort(char *layout, int r, int &port)
{
	int lerr = 0, gerr;

	std::cerr << "looking for r = " << r << " in " << layout << "\n";

  string server;
  ifstream ifs(layout);
  for (int i = 0; !lerr && i <= mpir; i++)
  {
    ifs >> server >> port;
    if (ifs.eof())
      lerr = 1;
  }

	std::cerr << "lerr " << lerr << "\n";

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
  int   nt = 1;
  char  *layoutfile = NULL;
  char  *tmplate = NULL;
	int   datatype;

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
        default: 
          syntax(argv[0]);
      }
    else
      syntax(argv[0]);

  if (! layoutfile || ! tmplate)
    syntax(argv[0]);

	if (! strcmp(tmplate + strlen(tmplate) -3, "vti"))
		datatype = VTI;
	else if (! strcmp(tmplate + strlen(tmplate) -3, "vtu"))
		datatype = VTU;
	else
	{
		if (mpir == 0)
			cerr << "Only .vtu and .vti files suported\n";
    MPI_Finalize();
    exit(1);
	}

	vector<vtkDataSet *> timesteps;

	for (int t = 0; t < nt; t++)
	{
		char filename[1024];
		sprintf(filename, tmplate, t, mpir);
		if (datatype == VTI)
			timesteps.push_back(ReadAnXMLFile<vtkXMLImageDataReader>(filename));
		else
			timesteps.push_back(ReadAnXMLFile<vtkXMLUnstructuredGridReader>(filename));
	}

	MPI_Barrier(MPI_COMM_WORLD);
	if (mpir == 0)
		cerr << "data loaded\n";

	int port;
  GetPort(layoutfile, mpir, port);

	MyServerSocket *serverSocket = MyServerSocket::New();
	serverSocket->CreateServer(port);

	vtkDataSetWriter *wr = vtkDataSetWriter::New();
	wr->WriteToOutputStringOn();

  for (int t = 0; t < nt; t++)
  {
    if (mpir == 0)
      std::cerr << "starting timestep " << t << "\n";

    vtkSocket *skt = OpenClientSocket(serverSocket, mpir);
	
    if (mpir == 0)
      std::cerr << "got socket\n";

		// If more than writing the raw data is required, a VTK pipeline 
		// would be inserted here

		wr->SetInputData(timesteps[t]);
		wr->Update();
    
		int sz = wr->GetOutputStringLength();
		void *ptr = wr->GetOutputString();

		skt->Send((const void *)&sz, sizeof(sz));
		skt->Send((const void *)ptr, sz);

		wr->Delete();

		skt->CloseSocket();
		skt->Delete();

    if (mpir == 0)
      std::cerr << "finished timestep " << t << "\n";
  }

	vtkSocket *skt = OpenClientSocket(serverSocket, mpir);
	int sz = -1;
	skt->Send((const void *)&sz, sizeof(sz));
		

	serverSocket->CloseSocket();
	serverSocket->Delete();

  MPI_Finalize();
}
