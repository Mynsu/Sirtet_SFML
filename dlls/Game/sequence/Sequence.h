#pragma once
#pragma hdrstop
#include "../Common.h"
#include <type_traits>
#include "ISequence.h"

namespace sequence
{
	class GAME_API Sequence
	{
	public:
		Sequence( ) = delete;
		Sequence( sf::RenderWindow& window );
		~Sequence( )
		{
			IsInstanciated = false;
		};
		Sequence( const Sequence& ) = delete;
		void operator=( const Sequence& ) = delete;

		void update( );
		void draw( );
	private:
		// Be careful of what 'MainSequenceType' is.  It's not enum class type.
		template < typename MainSequenceType,
			std::enable_if_t< std::is_base_of_v< ::sequence::ISequence, std::decay_t< MainSequenceType > > >* = nullptr >
		void moveTo( )
		{
			mCurrentSequence.reset( nullptr );
			mCurrentSequence = std::make_unique< MainSequenceType >( mWindow, mNextMainSequence );
			if ( ::sequence::Seq::NONE != *mNextMainSequence )
			{
				const std::string typeName( typeid( MainSequenceType ).name( ) );
				::global::Console( )->printError( ErrorLevel::WARNING,
												  "Sequence transition just after " + typeName );
				*mNextMainSequence = ::sequence::Seq::NONE;
			}
		}
		// A single instance can live at a time, two or more can't.
		static bool IsInstanciated;
		sf::RenderWindow& mWindow;
		// IMPORTANT: Please regard this type as '::sequence::Seq* const.'
		::sequence::Seq* mNextMainSequence;
		std::unique_ptr< ::sequence::ISequence > mCurrentSequence;
	};
}