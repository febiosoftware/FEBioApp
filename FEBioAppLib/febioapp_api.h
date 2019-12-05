#pragma once

#ifdef WIN32
	#ifdef FEBIO_APP_EXPORTS
		#define FEBIO_APP_API __declspec(dllexport)
	#else
		#define FEBIO_APP_API __declspec(dllimport)
	#endif
#endif
