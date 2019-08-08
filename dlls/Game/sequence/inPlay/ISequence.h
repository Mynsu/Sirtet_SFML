#pragma once
#pragma hdrstop
#include "../../Common.h"

namespace sequence::inPlay
{
	enum class Seq
	{
		READY = 0,
		PLAYING,
		NONE,
	};

	///class GAME_API ISequence
	class ISequence
	{
	public:
		ISequence( const ISequence& ) = delete;
		void operator=( const ISequence& ) = delete;
		virtual ~ISequence( ) = default;

		virtual void update( ) = 0;
		virtual void draw( ) = 0;
		// NOTE: Protected constructor prevents users
		// from instantiating the abstract class intended to be like INTERFACE in Java.
	protected:
		ISequence( ) = default;
	};
}