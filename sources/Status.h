#pragma once
#include "fsal_common.h"

namespace fsal
{
	struct Status
	{
		enum State : uint8_t
		{
			kOK = 0,
			kEOF = 1 << 0,
			kFailed = 1 << 1
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

	inline Status::State operator | (Status::State a, Status::State b)
	{
		return (Status::State)(uint8_t(a) | uint8_t(b));
	}

	inline Status::State operator |= (Status::State& a, Status::State b)
	{
		a = a | b;
		return a;
	}

	inline Status::State operator & (Status::State a, Status::State b)
	{
		return (Status::State)(uint8_t(a) & uint8_t(b));
	}

	inline Status::State operator &= (Status::State& a, Status::State b)
	{
		a = a & b;
		return a;
	}
}
