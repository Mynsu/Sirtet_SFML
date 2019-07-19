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

	class GAME_API ISequence
	{
	public:
		ISequence( ) = default;
		ISequence( const ISequence& ) = delete;
		void operator=( const ISequence& ) = delete;
		virtual ~ISequence( ) = default;

		virtual void update( ) = 0;
		virtual void draw( ) = 0;
	};
}