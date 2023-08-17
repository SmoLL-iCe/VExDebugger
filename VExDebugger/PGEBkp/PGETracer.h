#pragma once
#include <windows.h>
#include <vector>
#include "PGE.hpp"

namespace PGETracer
{
#ifdef USE_SWBREAKPOINT
	bool ResolverMultiplesSwBrkpt( EXCEPTION_RECORD* pException );
#endif
	bool ManagerCall( EXCEPTION_POINTERS* pExceptionInfo, StepBkp& Step, std::vector<PageGuardException>::iterator PGEit );
	bool ManagerCall2( EXCEPTION_POINTERS* pExceptionInfo, StepBkp& Step, std::vector<PageGuardException>::iterator PGEit );
}