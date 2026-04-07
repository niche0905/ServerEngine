#include "pch.h"
#include "Service/Server/TimeThiefServerApp.h"

	  //---------------------------//
	 //		Time Thief Server	  //	◞‸◟
	//---------------------------//
int main(int argc, char* argv[])//
  //---------------------------//
{
	TimeThiefServerApp app;
	
	if (!app.Init(argc, argv))
		return 1;
	
	if (!app.Start()) {
		app.Shutdown();
		return 1;
	}
	
	app.Run();
	app.Shutdown();
	return 0;
}
