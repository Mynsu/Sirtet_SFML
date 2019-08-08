#pragma once
#pragma hdrstop
#include "../Common.h"

namespace sequence
{
	// NOTE: GAME_API, or __declspec( dllimport/dllexport ) is unnecessary.
	enum class Seq
	{
		INTRO = 0,
		MAIN_MENU,
		SINGLE_PLAY,
		MULTI_PLAY,
		MAX_NONE,
	};

	using SetSequence_t = const std::function< void( const ::sequence::Seq ) >;

	class ISequence
	{
	public:
		ISequence( const ISequence& ) = delete;
		void operator=( const ISequence& ) = delete;
		virtual ~ISequence( ) = default;

		virtual void update( ) = 0;
		virtual void draw( ) = 0;
		//virtual auto newInstanceOfEqualType( ) -> std::unique_ptr< ISequence >&& = 0;//TODO

		// NOTE: Protected constructor prevents users
		// from instantiating the abstract class intended to be like INTERFACE in Java.
	protected:
		ISequence( ) = default;
	};
}
