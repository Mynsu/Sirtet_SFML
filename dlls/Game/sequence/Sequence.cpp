#include "Sequence.h"
#include <Lib/ScriptLoader.h>
#include "../Common.h"
#include "Intro.h"
#include "MainMenu.h"
//#include "inPlay/InPlay.h"

namespace
{
	// At a time lives a single instance of this type, two or more can't.
	// NOTE: Global access isn't necessary here.
	bool IsInstanciated = false;
}

void sequence::Sequence::lastInit( sf::RenderWindow* const window )
{
	mWindow = window;

#ifdef _DEV
	constexpr HashedKey HK_COMMAND0 = ::util::hash::Digest( "chseqto" );
	ServiceLocatorMirror::Console( )->addCommand( HK_COMMAND0, std::bind( &Sequence::_2436549370, this, std::placeholders::_1 ) );
	constexpr HashedKey HK_COMMAND1 = ::util::hash::Digest( "refresh" );
	ServiceLocatorMirror::Console( )->addCommand( HK_COMMAND1, std::bind( &Sequence::_495146883, this, std::placeholders::_1 ) );

	::sequence::Seq startSequence = ::sequence::Seq::INTRO;
	const std::string scriptPathNName( "Scripts/_OnlyDuringDev.lua" );
	const std::string varName( "StartMainSequence" );
	const auto result = ::util::script::LoadFromScript( scriptPathNName, varName );
	// When the variable 'StartMainSequence' exists in the script,
	if ( const auto it = result.find( varName ); result.cend( ) != it )
	{
		// When its type is interger which can be cast to enum type,
		if ( true == std::holds_alternative< int >( it->second ) )
		{
			startSequence = static_cast< ::sequence::Seq >( std::get< int >( it->second ) );
		}
		// Exception: When its type isN'T integer,
		else
		{
			ServiceLocatorMirror::Console( )->printScriptError( FailureLevel::WARNING, varName, scriptPathNName );
		}
	}
	// Exception: When the variable 'StartMainSequence' doesN'T exist in the script,
	else
	{
		ServiceLocatorMirror::Console( )->printScriptError( FailureLevel::WARNING, varName, scriptPathNName );
	}
	setSequence( startSequence );
#else
	mCurrentSequence = std::make_unique< ::sequence::Intro >( *window, std::move( mSetter ) );
#endif
}

void sequence::Sequence::setSequence( const ::sequence::Seq nextSequence )
{
	ASSERT_NOT_NULL( mWindow );

	mCurrentSequence.reset( nullptr );
	switch ( nextSequence )
	{
		case ::sequence::Seq::INTRO:
			mCurrentSequence = std::make_unique< ::sequence::Intro >( *mWindow, mSetter );
			break;
		case ::sequence::Seq::MAIN_MENU:
			mCurrentSequence = std::make_unique< ::sequence::MainMenu >( *mWindow, mSetter );
			break;
		case ::sequence::Seq::SINGLE_PLAY:
			//mCurrentSequence = std::make_unique< ::sequence::inPlay::InPlay >( *mWindow, mSetter );
			//break;
		case ::sequence::Seq::MULTI_PLAY:
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
}

void sequence::Sequence::_2436549370( const std::string_view& args )
{
	setSequence( static_cast< ::sequence::Seq >( std::atoi( args.data( ) ) ) );
}

void sequence::Sequence::_495146883( const std::string_view& )
{
	::sequence::ISequence* temp = mCurrentSequence.release( );
	mCurrentSequence = temp->newEqualTypeInstance( );
	delete temp;
	temp = nullptr;
}
