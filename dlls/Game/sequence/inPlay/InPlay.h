#pragma once
#pragma hdrstop
#include "../../Common.h"
#include <type_traits>
#include "../ISequence.h"
#include "ISequence.h"

namespace sequence::inPlay
{
	class GAME_API InPlay final : public ::sequence::ISequence
	{
	public:
		InPlay( ) = delete;
		InPlay( sf::RenderWindow& window,
				::sequence::Seq* const nextMainSequence );
		~InPlay( )
		{
			IsInstanciated = false;
		}

		void update( ) override;
		void draw( ) override;
	private:
		// Be careful of what 'InPlaySequenceType' is.  It's not enum class.
		template < typename InPlaySequenceType,
			std::enable_if_t< std::is_base_of_v< ::sequence::inPlay::ISequence, std::decay_t< InPlaySequenceType > > >* = nullptr >
		void moveTo( )
		{
			mCurrentInPlaySequence.reset( );
			mCurrentInPlaySequence = std::make_unique< InPlaySequenceType >( mWindow, mNextInPlaySequence, mBackgroundRect );
#ifdef _DEBUG
			if ( ::sequence::inPlay::Seq::NONE != *mNextInPlaySequence )
			{
				const std::string typeName( typeid( InPlaySequenceType ).name( ) );
				::global::Console( )->printError( ErrorLevel::WARNING,
												  "Sequence transition just after " + typeName );
				*mNextInPlaySequence = ::sequence::inPlay::Seq::NONE;
			}
#endif
		}
		// A single instance can live at a time, two or more can't.
		// NOTE: Singleton pattern and service locator pattern also give this,
		//		 but global access isn't necessary here.
		static bool IsInstanciated;
		sf::RenderWindow& mWindow;
		::sequence::Seq* const mNextMainSequence;
		// IMPORTANT: Please regard this type as '::sequence::inPlay::Seq* const.'
		::sequence::inPlay::Seq* mNextInPlaySequence;
		std::unique_ptr< ::sequence::inPlay::ISequence > mCurrentInPlaySequence;
		sf::RectangleShape mBackgroundRect;
	};
}