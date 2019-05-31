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
		Sequence( sf::RenderWindow& window );
		~Sequence( )
		{
			IsInstanciated = false;
		};
		//TODO: 복사 생성자, 연산자 delete

		void update( );
		void draw( );
	private:
		// Be careful of what 'MainSequenceType' is.  It's not enum class.
		template < typename MainSequenceType >
		void moveTo( )
		{
			static_assert( std::is_base_of< ::sequence::ISequence, MainSequenceType>( ) );
			mCurrentSequence.reset( nullptr );
			mCurrentSequence = std::make_unique< MainSequenceType >( mWindow, mNextMainSequence );
#ifdef _DEBUG
			if ( ::sequence::Seq::NONE != *mNextMainSequence )
			{
				const std::string typeName( typeid( MainSequenceType ).name( ) );
				::global::Console( )->printError( "Sequence Transition Warning: " + typeName );
				*mNextMainSequence = ::sequence::Seq::NONE;
			}
#endif
		}
		// A single instance can live at a time, two or more can't.
		static bool IsInstanciated;
		sf::RenderWindow& mWindow;
		// IMPORTANT: Please regard this type as '::sequence::Seq* const.'
		::sequence::Seq* mNextMainSequence;
		std::unique_ptr< ::sequence::ISequence > mCurrentSequence;
	};
}