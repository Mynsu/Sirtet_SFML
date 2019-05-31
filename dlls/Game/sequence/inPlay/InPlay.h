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
		template < typename InPlaySequenceType >
		void moveTo( )
		{
			static_assert( std::is_base_of< ::sequence::inPlay::ISequence, InPlaySequenceType >( ) );
			mCurrentInPlaySequence.reset( );
			mCurrentInPlaySequence = std::make_unique< InPlaySequenceType >( iWindow, mNextInPlaySequence );
#ifdef _DEBUG
			if ( ::sequence::inPlay::Seq::NONE != *mNextInPlaySequence )
			{
				const std::string typeName( typeid( InPlaySequenceType ).name( ) );
				::global::Console( )->printError( "Sequence Transition Warning: " + typeName );
				*mNextInPlaySequence = ::sequence::inPlay::Seq::NONE;
			}
#endif
		}
		static bool IsInstanciated;
		// IMPORTANT: Please regard this type as '::sequence::inPlay::Seq* const.'
		::sequence::inPlay::Seq* mNextInPlaySequence;
		std::unique_ptr< ::sequence::inPlay::ISequence > mCurrentInPlaySequence;
	};
}