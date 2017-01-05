#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <iostream>
#include <string>
#include <vtkSmartPointer.h>
#include <vtkCharArray.h>
#include <vtkXMLDataSetWriter.h>
#include <vtkDataSetReader.h>
#include <vtkClientSocket.h>
#include <vtkServerSocket.h>

using namespace vtk;
using namespace std;

int mpir, mpis;


void
syntax(char *a)
{
	if (mpir == 0)
	{
    cerr << "syntax: " << a << " template layoutfile [options]\n";
    cerr << "options:\n";
	}
	MPI_Finalize();
	exit(1);
}

void
GetServerAndPort(char *layout, int r, string& server, int& port)
{
  int lerr = 0, gerr;

  ifstream ifs(layout);
  for (int i = 0; !lerr && i <= r; i++)
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

int
main(int argc, char *argv[])
{
	char *layoutfile = NULL;
	char *tmplate = NULL;

	MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &mpis);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpir);

	tmplate = argv[1];
	layoutfile = argv[2];

	for (int i = 3; i < argc; i++)
    if (argv[i][0] == '-')
      switch(argv[i][1])
      {
        default:
          syntax(argv[0]);
      }
    else
      syntax(argv[0]);

  if (! layoutfile || ! tmplate)
    syntax(argv[0]);

	string server;
	int port;

	GetServerAndPort(layoutfile, mpir, server, port);

  vtkClientSocket *skt = vtkClientSocket::New();
  int gerr, lerr = skt->ConnectToServer(server.c_str(), port);

  MPI_Allreduce(&lerr, &gerr, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
  if (gerr)
  {
    if (mpir == 0) cerr << "all clients can't reach their server\n";
    MPI_Finalize();
    exit(1);
  }

  int tstep = 0;
  while (true)
  {
    int flag;
    if ((mpir == 0) && (skt->Receive(&flag, sizeof(flag)) < sizeof(flag)))
        flag = 0;

    MPI_Bcast(&flag, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (! flag)
      break;

		int sz;
		char *str;

		skt->Receive((void *)&sz, sizeof(sz));
		if (sz < 0) break;
	
		vtkSmartPointer<vtkCharArray> array = vtkSmartPointer<vtkCharArray>::New();
		array->Allocate(sz);
		array->SetNumberOfTuples(sz);
		void *ptr = array->GetVoidPointer(0);

		skt->Receive(ptr, sz);
	
		vtkSmartPointer<vtkDataSetReader> reader = vtkSmartPointer<vtkDataSetReader>::New();
		reader->ReadFromInputStringOn();
		reader->SetInputArray(array.GetPointer());
		reader->Update();

		vtkSmartPointer<vtkXMLDataSetWriter> writer = vtkSmartPointer<vtkXMLDataSetWriter>::New();
		writer->SetInputConnection(reader->GetOutputPort());

		char filename[1024];
		sprintf(filename, tmplate, tstep++, mpir);
		writer->SetFileName(filename);

		writer->Update();
	}

  skt->CloseSocket();
  skt->Delete();

	MPI_Finalize();
	exit(0);
}
