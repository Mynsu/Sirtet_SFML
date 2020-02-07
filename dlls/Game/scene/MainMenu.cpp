#include "../pch.h"
#include "MainMenu.h"
#include "../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"

bool ::scene::MainMenu::IsInstantiated = false;

::scene::MainMenu::MainMenu( sf::RenderWindow& window )
	: mIsCursorOnButton( false ), mWindow_( window ),
	mNextSceneID( ::scene::ID::AS_IS )
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
	std::string spritePathNName( "Images/MainMenu.png" );
	mSpriteClipSize_ = sf::Vector2f(256.f, 128.f);
	mLogoMargin_ = sf::Vector2f(70.f, 70.f);
	mButtonSinglePosition_ = sf::Vector2f(150.f, 150.f);
	mButtonOnlinePosition_ = sf::Vector2f(150.f, 300.f);
	mAudioList[(int)AudioIndex::BGM] = "Audio/korobeiniki.mp3";
	mAudioList[(int)AudioIndex::ON_SELECTION] = "Audio/selection.wav";

	lua_State* lua = luaL_newstate( );
	const std::string scriptPathNName( "Scripts/MainMenu.lua" );
	if ( true == luaL_dofile(lua, scriptPathNName.data()) )
	{
		// File Not Found Exception
		gService( )->console( ).printFailure( FailureLevel::FATAL, "File Not Found: "+scriptPathNName );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;
		std::string tableName( "Sprite" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPathNName );
		}
		else
		{
			std::string field( "path" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				spritePathNName = lua_tostring(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "clipWidth";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				if ( 0 > temp )
				{
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
																		tableName+':'+field, scriptPathNName );
				}
				else
				{
					mSpriteClipSize_.x = temp;
				}
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "clipHeight";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				if ( 0 > temp )
				{
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
																		tableName+':'+field, scriptPathNName );
				}
				else
				{
					mSpriteClipSize_.y = temp;
				}
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );

		tableName = "LogoMargin";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPathNName );
		}
		else
		{
			std::string field( "x" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				if ( 0 > temp )
				{
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
																		tableName+':'+field, scriptPathNName );
				}
				else
				{
					mLogoMargin_.x = temp;
				}
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				if ( 0 > temp )
				{
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
																		tableName+':'+field, scriptPathNName );
				}
				else
				{
					mLogoMargin_.y = temp;
				}
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );

		tableName = "ButtonSingle";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPathNName );
		}
		else
		{
			std::string field( "x" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				if ( 0 > temp )
				{
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
																		tableName+':'+field, scriptPathNName );
				}
				else
				{
					mButtonSinglePosition_.x = temp;
				}
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				if ( 0 > temp )
				{
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
																		tableName+':'+field, scriptPathNName );
				}
				else
				{
					mButtonSinglePosition_.y = temp;
				}
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );

		tableName = "ButtonOnline";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPathNName );
		}
		else
		{
			std::string field( "x" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber( lua, TOP_IDX );
				if ( 0 > temp )
				{
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
															tableName+':'+field, scriptPathNName );
				}
				else
				{
					mButtonOnlinePosition_.x = temp;
				}
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				if ( 0 > temp )
				{
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
															tableName+':'+field, scriptPathNName );
				}
				else
				{
					mButtonOnlinePosition_.y = temp;
				}
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );

		tableName = "Audio";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPathNName );
		}
		else
		{
			std::string innerTableName( "BGM" );
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+innerTableName, scriptPathNName );
			}
			else
			{
				std::string field( "path" );
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				int type = lua_type(lua, TOP_IDX);
				if ( LUA_TSTRING == type )
				{
					mAudioList[(int)AudioIndex::BGM] = lua_tostring(lua, TOP_IDX);
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+field, scriptPathNName );
				}
				else
				{
					gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );
			}
			lua_pop( lua, 1 );

			innerTableName = "onSelection";
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+innerTableName, scriptPathNName );
			}
			else
			{
				std::string field( "path" );
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				int type = lua_type(lua, TOP_IDX);
				if ( LUA_TSTRING == type )
				{
					mAudioList[(int)AudioIndex::ON_SELECTION] = lua_tostring(lua, TOP_IDX);
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+field, scriptPathNName );
				}
				else
				{
					gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );
	}
	lua_close( lua );

	if ( false == mTexture.loadFromFile(spritePathNName) )
	{
		// Exception: When there's not even the default file,
		gService( )->console( ).printFailure( FailureLevel::FATAL, "File Not Found: "+spritePathNName );
#ifdef _DEBUG
		__debugbreak( );
#endif
	}
	mSprite.setTexture( mTexture );
	if ( false == gService()->audio().playBGM(mAudioList[(int)AudioIndex::BGM], true) )
	{
		gService()->console().printFailure(FailureLevel::WARNING,
										   "File Not Found: "+mAudioList[(int)AudioIndex::BGM] );
	}
}

::scene::ID scene::MainMenu::update( std::vector<sf::Event>& eventQueue )
{
	for ( const auto& it : eventQueue )
	{
		if ( sf::Event::KeyPressed == it.type &&
			sf::Keyboard::Escape == it.key.code )
		{
			auto& vault = gService()->vault();
			const auto it = vault.find(HK_IS_RUNNING);
#ifdef _DEBUG
			if ( vault.end() == it )
			{
				__debugbreak( );
			}
#endif
			it->second = 0;
		}
	}

	return mNextSceneID;
}

void ::scene::MainMenu::draw( )
{
	const sf::Vector2f winSize( mWindow_.getSize( ) );
	// Bottom right on screen
	const sf::Vector2f logoSize( mSpriteClipSize_.x, 2*mSpriteClipSize_.y );
	mSprite.setPosition( winSize - logoSize - mLogoMargin_ );
	mSprite.setTextureRect( sf::IntRect(sf::Vector2i(0,0), sf::Vector2i(logoSize)) );
	mWindow_.draw( mSprite );
	
	touchButton();
}

#ifdef _DEV
::scene::ID scene::MainMenu::currentScene( ) const
{
	return ::scene::ID::MAIN_MENU;
}
#endif

void scene::MainMenu::touchButton( )
{
	const sf::Vector2f mouseGlobalPos( sf::Mouse::getPosition( ) );
	const sf::Vector2i cast( mSpriteClipSize_ );
	const sf::Vector2f titlebarHeight( 0.f, 20.f );
		
	bool hasGainedFocus = false;
	auto& vault = gService()->vault();
	if ( const auto it = vault.find(HK_HAS_GAINED_FOCUS);
		vault.end() != it )
	{
		hasGainedFocus = (bool)it->second;
	}
		
	if ( true == hasGainedFocus && false == gService()->console( ).isVisible() )
	{
		if ( const sf::Vector2f btSingleGlobalPos( sf::Vector2f(mWindow_.getPosition())+titlebarHeight+mButtonSinglePosition_ ); 
			 btSingleGlobalPos.x < mouseGlobalPos.x && mouseGlobalPos.x < btSingleGlobalPos.x+mSpriteClipSize_.x
			 && btSingleGlobalPos.y < mouseGlobalPos.y && mouseGlobalPos.y < btSingleGlobalPos.y+mSpriteClipSize_.y )
		{
			if ( true == sf::Mouse::isButtonPressed(sf::Mouse::Left) )
			{
				mNextSceneID = ::scene::ID::SINGLE_PLAY;
			}
			mSprite.setPosition( mButtonSinglePosition_ );
			mSprite.setTextureRect( sf::IntRect( cast.x,
												 2*cast.y, cast.x, cast.y ) );
			mWindow_.draw( mSprite );
			mSprite.setPosition( mButtonOnlinePosition_ );
			mSprite.setTextureRect( sf::IntRect( 0,
												 3*cast.y, cast.x, cast.y ) );
			mWindow_.draw( mSprite );
			if ( false == mIsCursorOnButton )
			{
				mIsCursorOnButton = true;
				if ( false == gService()->audio().playSFX(mAudioList[(int)AudioIndex::ON_SELECTION]) )
				{
					gService()->console().printFailure(FailureLevel::WARNING,
													   "File Not Found: "+mAudioList[(int)AudioIndex::ON_SELECTION] );
				}
			}
		}
		else if ( const sf::Vector2f btOnlineGlobalPos( sf::Vector2f(mWindow_.getPosition())+titlebarHeight+mButtonOnlinePosition_ );
				  btOnlineGlobalPos.x < mouseGlobalPos.x && mouseGlobalPos.x < btOnlineGlobalPos.x+mSpriteClipSize_.x
				  && btOnlineGlobalPos.y < mouseGlobalPos.y && mouseGlobalPos.y < btOnlineGlobalPos.y+mSpriteClipSize_.y )
		{
			if ( true == sf::Mouse::isButtonPressed(sf::Mouse::Left) )
			{
				mNextSceneID = ::scene::ID::ONLINE_BATTLE;
			}
			mSprite.setPosition( mButtonSinglePosition_ );
			mSprite.setTextureRect( sf::IntRect( 0,
												 2*cast.y, cast.x, cast.y ) );
			mWindow_.draw( mSprite );
			mSprite.setPosition( mButtonOnlinePosition_ );
			mSprite.setTextureRect( sf::IntRect( cast.x,
												 3*cast.y, cast.x, cast.y ) );
			mWindow_.draw( mSprite );
			if ( false == mIsCursorOnButton )
			{
				mIsCursorOnButton = true;
				if ( false == gService()->audio().playSFX(mAudioList[(int)AudioIndex::ON_SELECTION]) )
				{
					gService()->console().printFailure(FailureLevel::WARNING,
													   "File Not Found: "+mAudioList[(int)AudioIndex::ON_SELECTION] );
				}
			}
		}
		else
		{
			mNextSceneID = ::scene::ID::AS_IS;
			mSprite.setPosition( mButtonSinglePosition_ );
			mSprite.setTextureRect( sf::IntRect( 0,
												 2*cast.y, cast.x, cast.y ) );
			mWindow_.draw( mSprite );
			mSprite.setPosition( mButtonOnlinePosition_ );
			mSprite.setTextureRect( sf::IntRect( 0,
												 3*cast.y, cast.x, cast.y ) );
			mWindow_.draw( mSprite );
			mIsCursorOnButton = false;
		}
	}
	else
	{
		mNextSceneID = ::scene::ID::AS_IS;
		mSprite.setPosition( mButtonSinglePosition_ );
		mSprite.setTextureRect( sf::IntRect( 0,
											 2*cast.y, cast.x, cast.y ) );
		mWindow_.draw( mSprite );
		mSprite.setPosition( mButtonOnlinePosition_ );
		mSprite.setTextureRect( sf::IntRect( 0,
											 3*cast.y, cast.x, cast.y ) );
		mWindow_.draw( mSprite );
		mIsCursorOnButton = false;
	}
}
