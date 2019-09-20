#include "../pch.h"
#include "MainMenu.h"
#include "../ServiceLocatorMirror.h"

bool ::scene::MainMenu::IsInstantiated = false;

::scene::MainMenu::MainMenu( sf::RenderWindow& window, const SetScene_t& setScene )
	: mWindow( window ), mSetScene( setScene ),
	mOnIndicator( ::scene::ID::MAX_NONE ),
	mSpriteClipSize_( 256.f, 128.f ), mLogoMargin_( 70.f, 70.f ),
	mButtonSinglePosition_( 150.f, 150.f ), mButtonOnlinePosition_( 150.f, 300.f )
{
	ASSERT_FALSE( IsInstantiated );

	loadResources( );

	IsInstantiated = true;
}

::scene::MainMenu::~MainMenu( )
{
	IsInstantiated = false;
}

void scene::MainMenu::loadResources( )
{
	bool isPathDefault = true;
	bool isSprWDefault = true;
	bool isSprHDefault = true;
	bool isMargXDefault = true;
	bool isMargYDefault = true;
	bool isButtXDefault = true;
	bool isButtYDefault = true;

	lua_State* lua = luaL_newstate( );
	const char scriptPathNName[] = "Scripts/MainMenu.lua";
	if ( true == luaL_dofile(lua, scriptPathNName) )
	{
		// File Not Found Exception
		(*glpService).console( )->printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+scriptPathNName );
		lua_close( lua );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;
		const std::string tableName0( "Sprite" );
		lua_getglobal( lua, tableName0.data( ) );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0.data( ), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "path";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TSTRING == type )
			{
				if ( false == mTexture.loadFromFile(lua_tostring(lua, TOP_IDX)) )
				{
					// File Not Found Exception
					(*glpService).console( )->printScriptError( ExceptionType::FILE_NOT_FOUND,
																		(tableName0+":"+field0).data( ), scriptPathNName );
				}
				else
				{
					isPathDefault = false;
				}
			}
			// Type Check Exception
			else
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName0+":"+field0).data( ), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "width";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger(lua, TOP_IDX));
				// Range Check Exception
				if ( 0 > temp )
				{
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
																		(tableName0+":"+field1).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					mSpriteClipSize_.x = temp;
					isSprWDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName0+":"+field1).data( ), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "height";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger(lua, TOP_IDX));
				// Range Check Exception
				if ( 0 > temp )
				{
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
																		(tableName0+":"+field2).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					mSpriteClipSize_.y = temp;
					isSprHDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName0+":"+field2).data( ), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}

		const std::string tableName1( "LogoMargin" );
		lua_getglobal( lua, tableName1.data( ) );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName1.data( ), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "x";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger(lua, TOP_IDX));
				// Range Check Exception
				if ( 0 > temp )
				{
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
																		(tableName1+":"+field0).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					mLogoMargin_.x = temp;
					isMargXDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName1+":"+field0).data( ), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "y";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger(lua, TOP_IDX));
				// Range Check Exception
				if ( 0 > temp )
				{
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
																		(tableName1+":"+field1).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					mLogoMargin_.y = temp;
					isMargYDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName1+":"+field1).data( ), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}

		const std::string tableName2( "ButtonSingle" );
		lua_getglobal( lua, tableName2.data( ) );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName2.data( ), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "x";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger(lua, TOP_IDX));
				// Range Check Exception
				if ( 0 > temp )
				{
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
																		(tableName2+":"+field0).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					mButtonSinglePosition_.x = temp;
					isButtXDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName2+":"+field0).data( ), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "y";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger(lua, TOP_IDX));
				// Range Check Exception
				if ( 0 > temp )
				{
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
																		(tableName2+":"+field1).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					mButtonSinglePosition_.y = temp;
					isButtYDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName2+":"+field1).data( ), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}

		const std::string tableName3( "ButtonOnline" );
		lua_getglobal( lua, tableName3.data( ) );
		// Type Check Exception
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName3.data( ), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "x";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger( lua, TOP_IDX ));
				// Range Check Exception
				if ( 0 > temp )
				{
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
						(tableName3+":"+field0).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					mButtonOnlinePosition_.x = temp;
					isButtXDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
					(tableName3+":"+field0).data( ), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "y";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger( lua, TOP_IDX ));
				// Range Check Exception
				if ( 0 > temp )
				{
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
						(tableName3+":"+field1).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					mButtonOnlinePosition_.y = temp;
					isButtYDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
					(tableName3+":"+field1).data( ), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}
		lua_close( lua );
	}

	if ( true == isPathDefault )
	{
		const char defaultFilePathNName[ ] = "Images/MainMenu.png";
		if ( false == mTexture.loadFromFile(defaultFilePathNName) )
		{
			// Exception: When there's not even the default file,
			(*glpService).console( )->printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+defaultFilePathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}

	if ( true == isSprWDefault || true == isSprHDefault )
	{
		(*glpService).console( )->print( "Default: clip width 256, clip height 128" );
	}
	if ( true == isMargXDefault || true == isMargYDefault )
	{
		(*glpService).console( )->print( "Default: margin x 70, margin y 70" );
	}
	if ( true == isButtXDefault || true == isButtYDefault )
	{
		(*glpService).console( )->print( "Default: button x 150, button y 150" );
	}

	mSprite.setTexture( mTexture );
}

void ::scene::MainMenu::update( std::list< sf::Event >& eventQueue )
{
	for ( const auto& it : eventQueue )
	{
		if ( sf::Event::KeyPressed == it.type && sf::Keyboard::Escape == it.key.code )
		{
			constexpr HashedKey HK_IS_RUNNING = ::util::hash::Digest( "isRunning", ::util::hash::Measure("isRunning") );
			(*glpService).vault()[ HK_IS_RUNNING ] = 0;
		}
	}

	// NOTE: Here is no boundary check.
	if ( false == (*glpService).console( )->isVisible()
		 && true == sf::Mouse::isButtonPressed(sf::Mouse::Left) )
	{
		if ( ::scene::ID::SINGLE_PLAY == mOnIndicator )
		{
			mSetScene( mOnIndicator );
		}
		else if ( ::scene::ID::ONLINE_BATTLE == mOnIndicator )
		{
			mSetScene( mOnIndicator );
		}
	}
}

void ::scene::MainMenu::draw( )
{
	const sf::Vector2f winSize( mWindow.getSize( ) );
	// Bottom right on screen
	const sf::Vector2f logoSize( mSpriteClipSize_.x, 2*mSpriteClipSize_.y );
	mSprite.setPosition( winSize - logoSize - mLogoMargin_ );
	mSprite.setTextureRect( sf::IntRect(sf::Vector2i(0,0), sf::Vector2i(logoSize)) );
	mWindow.draw( mSprite );
	
	touchButton();
}

::scene::ID scene::MainMenu::currentScene( ) const
{
	return ::scene::ID::MAIN_MENU;
}

void scene::MainMenu::touchButton( )
{
	const sf::Vector2f mouseGlobalPos( sf::Mouse::getPosition( ) );
	const sf::Vector2i cast( mSpriteClipSize_ );
	const sf::Vector2f titlebarHeight( 0.f, 20.f );
	if ( const sf::Vector2f btSingleGlobalPos( sf::Vector2f(mWindow.getPosition())+titlebarHeight+mButtonSinglePosition_ ); 
		 btSingleGlobalPos.x < mouseGlobalPos.x && mouseGlobalPos.x < btSingleGlobalPos.x+mSpriteClipSize_.x
		 && btSingleGlobalPos.y < mouseGlobalPos.y && mouseGlobalPos.y < btSingleGlobalPos.y+mSpriteClipSize_.y )
	{
		mOnIndicator = ::scene::ID::SINGLE_PLAY;
		mSprite.setPosition( mButtonSinglePosition_ );
		mSprite.setTextureRect( sf::IntRect( cast.x,
											 2*cast.y, cast.x, cast.y ) );
		mWindow.draw( mSprite );
		mSprite.setPosition( mButtonOnlinePosition_ );
		mSprite.setTextureRect( sf::IntRect( 0,
											 3*cast.y, cast.x, cast.y ) );
		mWindow.draw( mSprite );
	}
	else if ( const sf::Vector2f btOnlineGlobalPos( sf::Vector2f(mWindow.getPosition())+titlebarHeight+mButtonOnlinePosition_ );
			  btOnlineGlobalPos.x < mouseGlobalPos.x && mouseGlobalPos.x < btOnlineGlobalPos.x+mSpriteClipSize_.x
			  && btOnlineGlobalPos.y < mouseGlobalPos.y && mouseGlobalPos.y < btOnlineGlobalPos.y+mSpriteClipSize_.y )
	{
		mOnIndicator = ::scene::ID::ONLINE_BATTLE;
		mSprite.setPosition( mButtonSinglePosition_ );
		mSprite.setTextureRect( sf::IntRect( 0,
											 2*cast.y, cast.x, cast.y ) );
		mWindow.draw( mSprite );
		mSprite.setPosition( mButtonOnlinePosition_ );
		mSprite.setTextureRect( sf::IntRect( cast.x,
											 3*cast.y, cast.x, cast.y ) );
		mWindow.draw( mSprite );
	}
	else
	{
		mOnIndicator = ::scene::ID::MAX_NONE;
		mSprite.setPosition( mButtonSinglePosition_ );
		mSprite.setTextureRect( sf::IntRect( 0,
											 2*cast.y, cast.x, cast.y ) );
		mWindow.draw( mSprite );
		mSprite.setPosition( mButtonOnlinePosition_ );
		mSprite.setTextureRect( sf::IntRect( 0,
											 3*cast.y, cast.x, cast.y ) );
		mWindow.draw( mSprite );
	}
}
