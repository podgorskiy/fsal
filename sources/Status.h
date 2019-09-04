#pragma once
#include "fsal_common.h"

namespace fsal
{
	struct Status
	{
		enum State
		{
			kOK,
			kEOF,
			kFailed
		};

		bool ok() const
		{
			return state == kOK;
		}

		Status() :state(kFailed)
		{
		}

		Status(State state) :state(state)
		{
		}

		Status(bool state) :state(state ? kOK : kFailed)
		{
		}
		
		operator bool() const
		{
			return ok();
		}
		
		State state;

		static Status Failed()
		{
			return Status(kFailed);
		}

		static Status Succeeded()
		{
			return Status(kOK);
		}

		static Status EndOfFile()
		{
			return Status(kEOF);
		}
	};
}
