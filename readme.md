Sirtet
======
[테트리스](https://tetris.fandom.com/wiki/Tetris_Guideline) 데모입니다.   
   
<a href="https://youtu.be/21LZqKT6xyA" target="_blank"><img src="https://user-images.githubusercontent.com/43042411/78128235-8cffc700-7450-11ea-990a-ffefc8c9c2bf.jpg" width="600" height="337" border="0"></a> *( 유투브 링크 )*


테트리스를 만든 이유는
----------------------
+ 게임과 코드가 면접관께 익숙하다면 심사하는 데 **피로가 덜할** 것이며,
+ [여기](https://www.gamedev.net/articles/programming/general-and-gameplay-programming/your-first-step-to-game-development-starts-here-r2976/)[저기](https://www.gamedev.net/articles/programming/general-and-gameplay-programming/how-do-i-make-games-a-path-to-game-development-r892/)에서 추천한 게임들 중 **게임 플레이 로직**이 제 머릿속에 그려지지 않았던 유일한 게임이고,
+ **하고 싶지 않은 게임이더라도** 끝까지 만들어 낼 수 있을지 제 자신이 궁금했기 때문입니다.

 
언어와 개발 환경은
------------------
C++ 17, Lua 5.3, 비주얼 스튜디오 2017(15.9) 그리고 윈도우즈 10(1803) x64입니다.


빌드와 실행에 필요한 외부 라이브러리와 운영체제는
----------------------------------------------
**NuGet**으로 관리하면 이슈가 있어 아래의 방법을 추천합니다.
+ \[클라이언트] Lua 5.3.0 이상
> 설치 예)
> 1. [다운로드 페이지](https://sourceforge.net/projects/luabinaries/files/5.3.5/Windows%20Libraries/Dynamic/)에서 *lua-5.3.5_Win64_dll15_lib.zip* 다운로드
> 2. *./externals/Lua-5.3/x64/* 에 *include* 폴더 압축 풀기
>   *./externals/Lua-5.3/x64/lib/* 에 *lua53.lib* 압축 풀기
> 3. *빌드된 Sirtet.exe와 Game.dll이 있는 디렉토리* 에 *lua53.dll* 압축 풀기
+ \[클라이언트/메인 서버] SFML 2.5.0 이상
> 설치 예)
> 1. [다운로드 페이지](https://www.sfml-dev.org/download/sfml/2.5.1/)에서 *Visual C++ 15 (2017) - 64-bit* 다운로드
> 2. *./externals/SFML-2.5/x64/* 에 *include* 폴더, *lib* 폴더 압축 풀기
> 3. *빌드된 Sirtet.exe와 Game.dll이 있는 디렉토리* 에 *openal32.dll, sfml-audio-2.dll, sfml-graphics-2.dll, sfml-system-2.dll, sfml-window-2.dll* 압축 풀기   
> *빌드된 Server.exe가 있는 디렉토리* 에 *sfml-graphics-2.dll, sfml-system-2.dll, sfml-window-2.dll* 압축 풀기   
> ( 디버그 모드로 빌드하시려면 *sfml-audio-**d**-2.dll*처럼 알파벳 **d**가 붙은 .dll 파일로 대체하십시오. )   
+ \[클라이언트/대기열 서버/메인 서버] 윈도우즈 7 이상입니다.


빌드하시기 전에!
----------------
**서버들의 IP 주소**가 localhost로 설정되어 있습니다.  *staticLibs/GameLib/Common.h*에서 수정하실 수 있습니다.   


IDE에서는 실행되는데 출력 디렉토리에서는 실행이 안 된다면
---------------------------------------------------------
**리소스 파일들이** 출력 디렉토리에 있어야 합니다.  이를 확인해주십시오.   
*./MakeSymbolicLinks.bat* 파일을 출력 디렉토리, 즉 *빌드된 Sirtet.exe와 Game.dll이 있는 디렉토리* 로 옮겨 실행하시면 편하게 리소스 파일들을 옮기실 수 있습니다.


실행하신 후에
-------------
+ BGM이 시끄러우시다면, **Ctrl + M**을 눌러주십시오.   
+ 콘솔을 켜시려면, **Tab 키**를 눌러주십시오.   
+ 씬 이동엔, **마우스와 Esc 키**를 써주십시오.   
+ **방향키와 스페이스 바**로 테트리미노를 움직이실 수 있습니다.


라이센스는 다음과 같습니다.
--------
+ [MIT](https://www.lua.org/license.html) (Lua)
+ [zlib/png](https://www.sfml-dev.org/license.php) (SFML)
+ [OFL](https://scripts.sil.org/OFL) (글꼴)


제 코드에 대해 궁금하실 만한 것들을 추렸습니다.
------------------------------------------------
**#깃 브랜칭 모형** **#깃 플로**   
  깃 플로를 따라 버전 관리했습니다.  feature 브랜치는 develop 브랜치에 squash 병합*merge*했습니다.   
  참고로, 도중에 깃 플로로 갈아탔기 때문에 master 브랜치에 어울리지 않는 커밋들이 조금 남아 있습니다.   

**#파일 디렉토리, 필터, 이름공간 일치**   
  예를 들어, 헤더 파일 *dlls/Game/scene/inPlay/InPlay.h*는 프로젝트 Game의 필터 scene/inPlay 아래에 있습니다.  클래스 InPlay의 이름공간도 ::scene::inPlay입니다.

**#단순한 코드 패턴** **#최소 작용의 원리(?)**   
  기능이 같은 한, 패턴이 단순하도록 코딩합니다.  패턴이 난잡하면 읽고 싶지 않은 것은 물론 오류*error*가 발생할 위험이 크기 때문입니다.   

**#헤더 파일을 포함*include*하는 순서** **#간접 포함에 기대지 않음**   
  C 표준*standard* 라이브러리, C++ 표준 라이브러리, 제 3자*3rd party* 라이브러리, 직접 만든 라이브러리, 나머지 순으로 포함합니다.   
  단, *WIN32_LEAN_AND_MEAN*을 정의한 다음 포함하면 컴파일 오류가 발생하는 헤더 파일은 가장 먼저 포함합니다.   
  필요한 헤더 파일이 이미 다른 헤더 파일을 통해 간접 포함되더라도 이에 의존하지 않고 직접 포함합니다.  간접 포함하면 헤더 파일을 뺄 때마다 필요한 헤더 파일까지 송두리채 빼진 않았는지 살펴야 하기 때문입니다.  개발 도중 컴파일러를 바꾸면 어떤 헤더를 여전히 간접 포함하는지 알 수 없기 때문이기도 합니다.   
( [각 프로젝트의 *pch.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/exes/Engine/pch.h) 등 )

 **#C++ 20 모듈은 아직** **#전방 선언*forward declaration***   
  가능한 한 소스 파일에만 헤더 파일을 포함합니다.  수정한 A.hpp를 B.hpp가 포함한다면, B.hpp를 포함하나 A.hpp는 쓰지 않는 소스 파일까지 괜히 다시 빌드하기 때문입니다.  그렇다고 Pimple 패턴은 쓰지 않았습니다.  읽기 번거롭기 때문입니다.   
  헤더 파일에 선언된 포인터와 레퍼런스를 타입 체크해야 하는 경우 그 타입을 다루는 헤더 파일을 포함하지 않고 타입을 전방 선언합니다.  이로써 순환 참조도 예방합니다.   
( [*dlls/Game/ui/NextTetriminoPanel.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/dlls/Game/ui/NextTetriminoPanel.h)와 [*PlayView.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/dlls/Game/ui/PlayView.h), [*dlls/Game/scene/online/*](https://github.com/Mynsu/Sirtet_SFML/tree/master/dlls/Game/scene/online) 아래 헤더 파일들, [*exes/Server/Room.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/exes/Server/Room.h) )

**#상수를 모아둔 헤더 파일**   
  공유하는 상수를 모아둔 헤더 파일은 있습니다만, 공유하지 않는 상수는 각 소스 파일에 두었습니다.  이유는 **#전방 선언**에서 헤더 파일을 되도록 소스 파일에만 포함하는 이유와 같습니다.   
( [*staticLibs/GameLib/Common.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/staticLibs/GameLib/Common.h) )

**#인터페이스** **#접두사 I** **#protected 생성자**   
  순수 가상 함수만 선언된 기반*base*·부모*parent*·상위·추상 클래스라는 이명들 대신 자바의 인터페이스라는 이름을 차용하겠습니다.  이 저장소에서 다른 의미의 '인터페이스'는 쓰지 않습니다.   
  인터페이스 이름 앞에 알파벳 대문자 I를 붙여 인스턴스를 만들지 않도록 유도하고, 상속이 아니라 구현*implement*이라는 것을 쉽게 파악할 수 있습니다.   
  인터페이스 생성자의 접근 한정자를 protected로 두어 인스턴스를 함부로 만들지 않도록 합니다.   
  단, 런타임 다형성은 남용하지 않도록 주의합니다.  힙에 간접 접근하고 병합으로 성능을 손해보기 때문입니다.   
( [*dlls/Game/scene/*](https://github.com/Mynsu/Sirtet_SFML/tree/master/dlls/Game/scene) 아래 *IScene.h*들 )

**#상속** **#컴포지트*composite***   
  상속은 인터페이스 구현에만 문법상 썼습니다.  다음과 같은 이유로 상속을 지양합니다.   
1. 하위 클래스와 상위 클래스가 'is-a' 관계라도 둘의 경계에 있는 속성, 변수가 있기 마련입니다.  이런 변수를 상위 클래스에서 하위 클래스로(혹은 그 반대로) 옮기려면 연쇄 효과를 감수해야 합니다.  그렇다고 하위 클래스에게 필요없는 상위 클래스의 변수를 그냥 놔두면 메모리와 캐시 라인을 낭비합니다.  게다가 필요없는 변수와 엮여 있는 함수도 노출됩니다.   
2. 하위 클래스에서 멤버 변수/함수를 선언할 때마다 상위 클래스를 살펴야 합니다.  [fragile base class](https://en.wikipedia.org/wiki/Fragile_base_class) 같은 사고가 발생할 수 있기 때문입니다.   
3. 'is-a' 관계에 줄곧 상속을 쓰다 보면 동시에 여러 클래스를 상속해야 하는 처지에 놓이게 됩니다.  다중 상속은 문제 1과 2가 더 심합니다.   
4. 코드를 반복하지 않기 위해 상속했다가 오히려 더 반복할 수 있습니다.   
  상속을 쓰지 않고는 유지보수하기 힘든 경우를 제외하고는 다른 좋은 대안이 있습니다.  굳이 'is-a' 관계를 코드에 나타내고 싶다면 클래스 이름, 이름공간, 파일 디렉토리로도 할 수 있습니다.  반복되는 코드는 모듈로 떼어내면 결합도를 낮추면서 여러 클래스에서도 쓸 수 있습니다.

**#키워드 final** **#devirtualize**   
  상속을 지양하므로 가능한 한 키워드 final을 꼭 붙여줍니다.  이는 컴파일러가 devirtualize하도록 돕는 역할도 합니다.   

**#키워드 virtual**   
  언젠가 상속될 수도 있는 클래스라도 미리 키워드 virtual을 붙여놓지 않습니다.  미리 붙여놓으면 괜히 vtable을 만들어 메모리를 낭비하기 때문입니다.   

**#키워드 override**   
  상위 클래스의 멤버 함수를 재정의하려는 멤버 함수엔 항상 키워드 override를 붙입니다.  이로써 키워드 virtual이 붙지 않은 멤버 함수를 재정의하려고 하거나 비슷한 시그니처 때문에 재정의했다고 착각하는 실수를 막을 수 있습니다.  순수 가상 함수를 정의한 멤버 함수에도 키워드 override를 붙여 구별했습니다.   
( [*dlls/Game/scene/*](https://github.com/Mynsu/Sirtet_SFML/tree/master/dlls/Game/scene) 아래 헤더 파일들 )

**#씬*scene* 계층** **#추상화**   
<img src="https://user-images.githubusercontent.com/43042411/76996394-ca664e00-6994-11ea-9f02-740c93d00dbd.png"></img>
  dynamic_cast를 쓰지 않아 씬 계층이 단순하고 게임 루프가 느리지 않습니다.   
  클래스 ::scene::inPlay::InPlay와 ::scene::online::Online은 ::scene::Intro와 ::scene::MainMenu처럼 인터페이스 ::scene::IScene의 구현이지만 씬이 아닙니다.  씬 매니저까지 빠져나와 처리할 필요가 없는 것들을 맡긴 간이 씬 매니저입니다.   
( [*dlls/Game/scene/*](https://github.com/Mynsu/Sirtet_SFML/tree/master/dlls/Game/scene) 아래 헤더 파일들 )

**#위임*delegation***   
  온라인 관련 씬만 담당하는 간이 씬 매니저 ::scene::online::Online은 자신이 다루는 씬들에게 자신의 기능을 위임합니다.  서버와 통신하기 위한 스레드와 전역변수를 소스 파일 *::scene::online::Online.cpp*에 선언, 정의했기 때문입니다.  일종의 정보 은닉이기도 합니다.   
  다만, 위임된 인스턴스를 지우거나 재귀 호출할 우려가 있습니다.   
( [*dlls/Game/scene/online/*](https://github.com/Mynsu/Sirtet_SFML/tree/master/dlls/Game/scene/online) 아래 헤더 파일들과 *Online.cpp* )

**#의존*dependency* 관계** **#편한 디버그** **#x64 호출 규약**   
  의존 관계를 선호합니다.  즉, 인스턴스의 레퍼런스를 멤버 변수로 항상 쥐고 있지 않고 필요할 때만 함수의 매개변수로 넘깁니다.  이렇게 하면 어떤 인스턴스가 어디에 쓰이는지 쉽게 보이기 때문에 디버그하기 수월합니다.  항상 쥐고 있지 않아도 되므로 메모리와 캐시 라인을 절약할 수도 있습니다.  x64 호출 규약*calling convention*상 4 개의 매개변수까진 레지스트리에 넘기므로 성능상 문제도 없습니다.   
  이 저장소에서 Aggregation 관계는 단 두 곳뿐입니다.  한 곳은 여러 씬들이 인스턴스 하나를 공유해야 하는데 씬들의 멤버 함수에 매개변수를 추가하려면 인터페이스까지 수정해야해서 생겼습니다.  다른 한 곳은 콘솔로 호출하는 함수들을 컨테이너 하나에 담기 위해 타입을 통일해야 해서, 매개변수가 서로 다르면 안되서 생겼습니다.   
( [*dlls/Game/scene/inPlay/* 아래 헤더 파일들에 선언된 sf::Drawable 멤버 레퍼런스](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/scene/inPlay/Ready.h#L20), [*exes/Engine/Command.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/exes/Engine/Command.h) )

**#매크로 상수** **#const와 constexpr**   
  디버그하기 힘들기 때문에 매크로 상수를 쓰지 않습니다.  대신 const나 constexpr로 상수를 만듭니다.  이들은 주소에 접근하지만 않으면 메모리에 상주하지 않아 메모리를 아낄 수도 있습니다.

**#전역 변수**   
  **#의존 관계**에서 설명하듯이 전역 변수를 지양합니다만, 로그, 콘솔, 사운드 등은 전역 변수로 다루는 게 편하고 디버그하기 어렵지 않습니다.   
  이 저장소에서는 전역 변수를 모아 관리하고 의존성 주입을 할 수 있는 서비스 로케이터 패턴을 썼습니다.  싱글턴 패턴은 쓰지 않았습니다.   
( [*exes/Engine/ServiceLocator.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/exes/Engine/ServiceLocator.h), [*dlls/Game/ServiceLocatorMirror.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/dlls/Game/ServiceLocatorMirror.h) )

**#가시성** **#접근 한정자**   
  구조체라도 public을 명시하고 클래스라도 private을 명시해, 어떤 접근한정자인지 생각하느라 조금이라도 피로가 쌓이지 않도록 합니다.   
  **#protected**도 참고해주십시오.

**#정보 은닉**   
  멤버 변수의 비const 레퍼런스를 반환하는 getter 함수는 setter 함수이기도 하다는 것에 주의했습니다.   

**#인스턴스 카운팅**   
  일부 클래스의 총 인스턴스 수를 제한했습니다.  이를 위해 싱글턴 패턴을 쓰지는 않았습니다.   
( [일부 헤더 파일의 static bool IsInstantiated](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/scene/Intro.h#L10) )
  
**#함수 이름** **#추상화**   
  함수 이름은 어떻게 동작하는지가 아니라 결국 하고자 하는 게 뭔지를 나타냅니다.  함수를 사용하는 사람은 어떻게 동작하는지 신경쓰고 싶어하지 않습니다.  어떻게 동작하는지 알려야 한다면 주석을 답니다.   
( [*dlls/Game/ui/PlayView.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/dlls/Game/ui/PlayView.h) )

**#함수의 입도*granularity***   
  함수 하나에 여러 기능을 넣었다가 이 중 하나만 필요할 때가 되어서야 함수를 쪼개기 시작하면 연쇄 효과를 감수해야 합니다.   

**#함수 ≠ 코드 번들**   
  단지 코드들을 묶어 놓자고 함수를 만들지는 않습니다.  불필요한 함수를 만들어 새 서브루틴에 접근하고 스택 프레임을 찍는 비용을 감수할 이유가 없습니다.  인라인 함수는 호출 비용이 없지만 10줄 안팎의 코드만 해당하고 헤더 파일이 불어나는 한계가 있습니다.  단지 코드들을 묶는 건 unnamed scope나 주석을 이용합니다.   

**#멤버 변수의 선언 순서** **#패딩** **#초기화 리스트*initializer list***   
  멤버 변수의 크기가 작은 것부터 선언하여 패딩을 줄입니다.  대용량 메모리가 흔해진 현실이지만 캐시 라인까지 낭비할 수는 없습니다.   
  초기화 리스트 순서 역시 선언 순서와 맞추어 컴파일러가 최적화하도록 유도했습니다.   

**#멤버 변수 이름** **#접두사** **#접미사**   
  멤버 변수의 이름 앞에 알파벳 소문자 m을 붙여 멤버 변수를 IDE에 의존하는 것보다 빠르게 식별할 수 있습니다.   
  멤버 변수가 레퍼런스거나 연산 반복을 피하기 위해 값을 임시로 저장해두는 경우 이름 뒤에 특수문자 \_를 붙였습니다.   

**#unnamed scope**   
  이름이 같은 지역 변수를 선언해도 컴파일러가 알아서 처리해주지만 unnamed scope로 지역 변수의 수명을 명시합니다.   

**#늦은*lazy* 지역 변수 선언**   
  지역 변수를 서브루틴의 머리에 모아 두지 않고 쓰이기 직전에 선언합니다.  머리에 모아두면 early exit할 때 지역 변수를 괜히 만든 꼴이 되기 때문입니다.  지역 변수가 쓰이는 곳과 떨어져서 선언하면 코드의 가독성도 떨어집니다.   

**#STL 원소에 임의 접근*random access***   
  std::array와 std::vector의 원소는 연산자 []로 접근합니다.  인덱스를 초과하는지 검사해주는 멤버 함수 at(...)은 쓰지 않고 손수 검사합니다.  at(...)은 C++ exception으로 예외 처리해서 연산자 []보다 느린데 이득은 크지 않기 때문입니다.  std::vector는 힙 할당과 병합 때문에 성능 손해가 있어 할 수 있다면 std::array를 이용합니다.   
  std::unordered_map의 원소는 멤버 함수 find(...)로 접근합니다.  없는 키를 넣었는데 값을 만들고 이 때문에 기본 생성자까지 강요하는 연산자 []는 되도록 쓰지 않습니다.  멤버 함수 at(...) 역시 쓰지 않습니다.   

**#std::array**   
  스택에 있는 배열을 받는 매개변수로 C 스타일 배열이 아닌 std::array를 둡니다.  크기와 차원을 전달하기 쉽기 때문입니다.

**#시간 복잡도 vs. 실제 성능**   
  단지 자료 구조 중간에 있는 원소를 빼내거나 새로 넣는 경우가 잦다고 해서 연결 리스트를 쓰진 않습니다.  원소의 크기가 64 바이트 미만이면 SIMD를 쓰지 않아도 배열이 더 빠릅니다.  이 경우에도 단일*singly* 연결 리스트는 여전히 배열보다 빠르긴 합니다만 자료 구조 특성상 쓰임새가 적습니다.   

**#문자열 리터럴**   
  문자열 리터럴은 (const/constexpr) char[]나 std::string으로 받습니다.  const char\*로 받은 문자열 리터럴은 바이너리 파일에 그대로 들어있어 보안에 해가 되기 때문입니다.

**#문자열을 받는 매개변수**   
  nullptr을 걸러낼 수 없는 char\*보다 const char (&)[N]을, 문자열 리터럴이 메모리에 남을 수 있는 const char (&)[N]보다 std::string을 선호합니다.  char\*이 쓰인 곳은 재귀 함수와 윈속 send 함수뿐입니다.   
( [*staticLibs/Lib/Hash.h:12*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/staticLibs/Lib/Hash.h#L12), [*Socket.h*:78](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/staticLibs/Lib/Socket.h#L78) )

**#문자열 해싱** **#보안** **#성능**   
  보안과 성능을 위해 문자열을 해싱해 다룹니다.  문자열을 해싱해 비교하면 ::strcmp(...) 등으로 비교하는 것보다 현저히 빠릅니다.

**#복사 생성자** **#이동 생성자** **#키워드 delete**   
  되도록 건들지 않되 복사/이동 생성자가 의도와 다르게 동작할 우려가 있으면 없애둡니다. 

**#범위 기반*range-based* for 반복문**   
  C 스타일 for 반복문보다 빠른 범위 기반 for 반복문을 선호합니다.  std::for_each(...)는 C 스타일 for 반복문보다 느려 단순 반복문으로 쓰기에 부적합합니다.  반복자*iterator*를 쓰는 for 반복문은 가장 느려 쓰지 않습니다.  범위 기반 for 반복문도 내부에서 반복자를 쓰는데도 불구하고 그 차이가 큽니다.   

**#enum class**   
  enum class 타입은 enum 타입과 달리 스코프*scope*를 명시하도록 강요하므로 더 안전하게 코딩할 수 있습니다.   

**#C 스타일 캐스트**   
  static_cast<T>, reinterpret_cast<T>를 쓰지 않고 (T), (T\*)처럼 C 스타일로 캐스트했습니다.  차이도 없고 읽고 쓰기도 쉽기 때문이었습니다.

**#switch 분기문**   
  if 분기문보다 switch 분기문을 선호합니다.  case들을 오름차순으로 정렬하면 컴파일러가 최적화해주기 때문입니다.   
  Default case가 있으면 안 되는 경우, __debugbreak()(x86의 __asm { int 3 })를 두어 디버그 모드에서 오류를 잡습니다.  디버그 모드에서 오류를 잡았으므로 릴리즈 모드에선 __assume(0)을 두어 컴파일러가 최적화하도록 유도합니다.

**#SFINAE** **#std::enable_if**   
  SFINAE로 템플릿이 특정 타입만 받도록 강제해봤습니다.  인텔리센스 덕분에 컴파일하지 않고도 추론이 어떤지를 알 수 있었습니다.  static_assert와 다른 점입니다.   
( [*staticLibs/Lib/ScriptLoader.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/staticLibs/Lib/ScriptLoader.h) )

**#RTTI**   
  굳이 쓰지 않아도 되고 성능 손해까지 있지만 연습 삼아 써봤습니다.   
( [*dlls/Game/scene/Playing.cpp:974* 키워드 typeid](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/scene/inPlay/Playing.cpp#L974), [*dlls/Game/scene/IScene.h:34* 순수 가상 함수 currentScene()의 정의](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/scene/Intro.cpp#L139) )

**#키워드 friend**   
  키워드 friend가 항상 정보 은닉을 깨는 건 아닙니다.   
( [*staticLibs/GameLib/IGame.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/staticLibs/GameLib/IGame.h), [*dlls/Game/ServiceLocatorMirror.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/dlls/Game/ServiceLocatorMirror.h)와 [*Game.cpp*](https://github.com/Mynsu/Sirtet_SFML/blob/master/dlls/Game/Game.cpp) )

**#goto**   
  두 겹 이상의 스코프를 빠져나가는 경우에만 goto를 허용했습니다.  키워드 break로는 할 수 없는데다 오히려 goto 문이 읽기도 쉽기 때문이었습니다.   
( [*dlls/Game/scene/MainMenu.cpp:999*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/scene/MainMenu.cpp#L999) )

**#예외 처리**   
  예외는 if 문으로 early exit하여 처리해 성능을 챙겼습니다.  C++ exception은 쓰지 않았습니다.   

**#NRVO**  
  불필요한 인스턴스 생성과 복사를 막기 위해 한 함수의 반환 값을 R-Value만으로 채우거나 Named Return Value만으로 채웁니다.   

**#인스턴스 복사 줄이기**   
  예를 들어 std::unordered_map은 멤버 함수 try_emplace(...)와 emplace(std::piecewise_construct, std::forward_as_tuple(...)...)를 써서 인스턴스가 불필요하게 복사되지 않도록 합니다.

**#인스턴스 재활용**   
  그래픽 인스턴스처럼 덩치가 크거나 소켓처럼 핸들을 얻는 데 비용이 큰 경우 가능한 한 기존 인스턴스를 재활용합니다.   
( [*dlls/Game/model/Tetrimino.h:118*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/model/Tetrimino.h#L118), [*dlls/Game/ui/NextTetriminoPanel.h:71*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/ui/NextTetriminoPanel.h#L71) )

**#헤더 파일만 있는 라이브러리**   
  헤더 파일만 공유하더라도 라이브러리를 통해 공유했습니다.  단순히 헤더 파일을 프로젝트마다 추가하면 인텔리센스가 구문 분석을 못하기 때문입니다.   
( [*staticLibs/GameLib/* 프로젝트](https://github.com/Mynsu/Sirtet_SFML/tree/master/staticLibs/GameLib) )

**#빌드 타임 링킹 라이브러리** **#정적 라이브러리**
  바뀐 코드가 없으면 .obj 파일을 그대로 써서 컴파일 시간을 절약합니다.  하지만 바꿀 일이 없는 코드를 아예 라이브러리로 떼어둠으로써 불필요한 연쇄 컴파일을 확실하게 막을 수 있습니다.  물론 커플링을 막고 여러 프로젝트에서 재사용하기에도 좋습니다.   
  여러 프로세스가 공유하는 라이브러리일 경우 빌드 타임 링킹은 독이 됩니다.  하나만 있으면 되는 라이브러리를 프로세스마다 가져 메모리를 낭비하기 때문입니다.  또, 출력 파일 .lib가 런타임 링킹의 출력 파일 .dll보다 수 배 이상 커지는 경향도 있습니다.   
( [*staticLibs/Lib/* 프로젝트](https://github.com/Mynsu/Sirtet_SFML/tree/master/staticLibs/Lib) )

**#런타임 링킹 라이브러리** **#.dll**   
  이 저장소에서 .dll 파일을 이용하는 이유는 여러 프로세스에서 공유하기 위해서가 아니라 Mod, 확장팩, 게임 컨텐츠를 엔진과 따로 개발하기 위해서입니다.   
  처음엔 .dll 파일을 .lib 파일을 통해 링킹했으나 더욱 추상화하기 위해 .lib 파일 없이 링킹하도록 수정했습니다.  이에 따라 .dll 파일을 런타임 중 해제/로드할 수 있게 되었지만 런타임 중 이럴 일은 없었습니다.   
( [*dlls/Game/* 프로젝트](https://github.com/Mynsu/Sirtet_SFML/tree/master/dlls/Game) )

**#씬 생성** **#의존성 주입 vs. 일원화**   
  처음엔 다음 씬을 생성하는 걸 각 씬에 맡겼습니다.  그런데 콘솔이나 스크립트로 원하는 씬을 열려면 씬 매니저도 씬을 생성할 수 있어야 했습니다.  두 방법을 혼용해보니 코드가 난잡해졌고 결국 씬 생성을 각 씬에 맡기지 않고 씬 매니저에게만 일임했습니다.  의존성 주입을 하지 않은 탓에 씬을 추가할 때마다 분기문을 써야 했습니다.   
( [*dlls/Game/scene/SceneManager.h*의 멤버 함수 update(...)](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/scene/SceneManager.h#L27)와 [setScene(...)](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/scene/SceneManager.cpp#L44) )

**#키워드 auto**   
  단지 타입 이름이 길다고 키워드 auto를 쓰진 않았습니다.  바로 근처 코드에서 쉽게 유추할 수 있어야 썼습니다.  코드를 읽는 데 방해되므로 남용하지 않습니다.  이 저장소에서는 반복자, structured binding, utf-8 문자열 리터럴을 받는 데 쓰였습니다.

**#Uniform 초기화**   
  Uniform 초기화는 매개변수가 std::initializer_list인 생성자에 따라서 의도치 않은 동작을 할 수 있기 때문에 쓰지 않습니다.  ( )로 초기화하는 것이 중의적인 경우 =로 초기화합니다.  이 저장소에서 { }로 초기화한 경우는 집합*aggregation* 초기화뿐입니다.

**#람다 표현식**   
  이 저장소에서 람다 표현식을 쓴 경우는 다음에 한합니다.  넘겨야 할 인자가 많은데 구조체나 투플로 묶어 넘기거나 전역으로 옮기는 것은 어색한 경우, 다른 스레드에서 돌릴 일회용 함수가 필요한 경우입니다.  어떤 변수가 캡쳐됐는지 알 수 있도록 [&], [=]처럼 자동으로 캡쳐하지 않고 하나하나 명시했습니다.   
( [*exes/QueueServer.cpp:269*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/exes/QueueServer/QueueServer.cpp#L269), [*Server.exe:124*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/exes/Server/Server.cpp#L124), [*dlls/Game/scene/online/Waiting.cpp:424*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/scene/online/Waiting.cpp#L424) )

**#루아*Lua* 스크립트**   
  기획자와 아티스트가 부담없이 수정할 수 있도록 스크립트를 이용하는 게임이 많습니다.  이 게임도 데이터와 리소스를 루아 스크립트로 다룹니다.   
로직은 빠르고 디버깅이 쉬워야 해서 스크립트가 아니라 네이티브 코드에서 다룹니다.

**#SFML**    
  SFML은 SDL처럼 오픈 소스 멀티미디어 라이브러리지만 레퍼런스가 적습니다.  그럼에도 SFML을 쓴 건 SDL과 달리 C++로 작성되었기 때문입니다.  C++와 OOP 코드의 좋은 예를 뜯어보며 보다 구체적으로 학습할 수 있었습니다.

**#콘솔**   
<a href="https://youtu.be/uPx0DgMIeoQ" target="_blank"><img src="https://user-images.githubusercontent.com/43042411/76969660-0508c000-696e-11ea-897a-5b9b868422ae.jpg" width="600" height="337" border="0"></a> *( 유투브 링크 )*   
  런타임 도중 여러 시도를 해볼 수 있도록 콘솔을 만들었습니다.  GUI가 구현되지 않았을 때도 콘솔로 기능을 실행해볼 수 있습니다.  현재는 스크립트 재적용, 원하는 씬으로 이동, 방 생성/입장 명령어들이 등록되어 있습니다.   
  주의할 점은 명령을 비동기로 수행하므로 공유 데이터를 관리해야 한다는 것입니다.   
( [*exes/Engine/Console.h*](https://github.com/Mynsu/Sirtet_SFML/blob/master/exes/Engine/Console.h) )

**#로그**   
  콘솔에만 출력하고 파일로는 출력하지 않습니다.

**#수학**   
  UI 애니메이션에 2차원 벡터 연산과 선형 보간이 쓰였습니다.   
( [*dlls/Game/scene/online/InLobby.cpp:952, 1375*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/scene/online/InLobby.cpp#L1375) )

**#TCP/IP**   
  이 게임은 TCP/IP를 이용합니다.  wouldblock 때 패킷이 유실될 수 있는 UDP는 이 게임에 쓸 일이 없었습니다.

**#대기열 서버** **#메인 서버**   
  서버로는 둘, 대기열 서버와 메인 서버가 있습니다.   
  둘 사이를 중개하는 Redis 서버나 계정 관리를 위한 RDBMS 서버는 없습니다.   
  클라이언트가 서버에 handshake하며 메시지를 동봉하더라도 서버가 메시지를 무시하도록 했습니다.  이렇게 하지 않으면 서버가 클라이언트의 메시지를 하염없이 기다리는 문제가 발생할 수 있기 때문이었습니다.      
<img src="https://user-images.githubusercontent.com/43042411/76689270-8f53da00-6677-11ea-9d8e-ad9e4df90409.png"></img>   
*( 접속 시 클라이언트와 서버들의 대화를 담은 시퀀스 다이어그램.  아래 3부터 해당. )*
1. 대기열 서버가 일반 클라이언트(혹은 피어*peer*)로서 메인 서버에 접속합니다.   
1-1. 대기열 서버가 멈추더라도 다시 켜 메인 서버에 접속할 수 있습니다.
2. 대기열 서버가 메인 서버에게 자신은 일반 클라이언트와 달리 대기열 서버임을 알립니다.
3. 클라이언트가 대기열 서버에 접속합니다.
4. 클라이언트가 대기열 서버에 미리 약속한 해시 값을 보냅니다.  대기열 서버는 해시 값, 즉 초대장을 보고 가짜 클라이언트가 아닌지, 서버들과 버전이 같은지를 확인합니다.
5. 클라이언트가 주어진 시간 내에 시퀀스 4를 통과하고 메인 서버에 빈자리까지 있으면, 대기열 서버가 새 티켓을 발급하여 클라이언트와 메인 서버 양측에 보냅니다.   
5-1-1. 클라이언트가 주어진 시간 내에 시퀀스 4를 통과했으나 메인 서버에 빈자리가 없으면, 대기열 서버는 클라이언트에 대기 번호를 발급하고 메인 서버에 빈자리가 났는지 주기적으로 물어봅니다.   
5-1-2. 메인 서버에 빈자리가 나면, 대기 번호 0 클라이언트에게 시퀀스 5를 수행합니다.   
5-2. 클라이언트가 주어진 시간 내에 시퀀스 4를 통과하지 못하면, 대기열 서버가 클라이언트의 접속을 끊고 시퀀스를 종료합니다.
6. 대기열 서버가 티켓을 보낸 클라이언트와 접속을 끊습니다.
7. 클라이언트가 메인 서버에 접속합니다.
8. 클라이언트가 대기열 서버로부터 받은 티켓을 메인 서버에 에코합니다.
9. 메인 서버가 클라이언트로부터 받은 티켓이 대기열 서버로부터 받은 티켓과 일치하면, 클라이언트의 접속을 유지합니다.   
9-1. 일치하지 않으면 메인 서버가 클라이언트의 접속을 끊습니다.

**#IOCP** **#Completion routine**   
  서버의 소켓 입출력에만 IOCP를 썼습니다.  게임 로직은 메인 스레드가 담당했습니다.  서버에서는 IOCP 때문에 completion routine을 비동기 호출할 수 없어 클라이언트에서만이라도 IOCP 대신 completion routine을 비동기 호출하도록 구현해봤습니다.   

**#백로그** **#::AcceptEx(...)**   
  백로그만큼 ::AcceptEx(...)를 걸어둡니다.   
  다만, 최근 걸어둔 소켓부터 accept가 완료되는 후입선출이기 때문에 TCP timed wait delay 상태에 있는 소켓을 걸어두지 않도록 유의해야 했습니다.   
( [*exes/QueueServer/QueueServer.cpp:245*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/exes/QueueServer/QueueServer.cpp#L245), [*exes/Server/Server.cpp:90*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/exes/Server/Server.cpp#L90) )

**#접속** **#Conditional accept** **#DDoS**   
  DDos를 예방하기 위해 소켓 옵션으로 conditional accept를 줘 서버가 accept를 완료해야 클라이언트가 접속할 수 있도록 했습니다.   
( [*exes/QueueServer/QueueServer.cpp:191*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/exes/QueueServer/QueueServer.cpp#L191), [*exes/Server/Server.cpp:46*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/exes/Server/Server.cpp#L46) )

**#소켓 재활용** **#DisconnectEx(...)**   
  **#인스턴스 재활용**과 연관됩니다.  O/S로부터 소켓 핸들을 얻는 비용이 크기 때문에 접속을 끊을 때 소켓을 버리지 않고 ::DisconnectEx(...)로 재활용합니다.  송수신을 걸어두다 실패한 소켓만 새것으로 교체합니다.  실패한 소켓들은 ::DisconnectEx(...)도 실패하기 때문에 다른 방법이 없습니다.   
  재활용하든 새것으로 교체하든 주의해야 할 것이 있습니다.  걸어 두었던 작업이 재활용하거나 새것으로 교체한 후 뒤늦게 이벤트를 알린다는 것입니다.  이것을 무시하도록 구현해야 합니다.

**#블로킹*Blocking* 소켓** **#멀티스레드** **#graceful shutdown**   
  클라이언트에는 블로킹 소켓을 썼습니다.  멀티스레드를 다뤄보고 싶었기 때문이었습니다.  클라이언트를 만들기 시작했을 때는 송수신 모두 블로킹이었으나 코드가 복잡하고 오작동이 잦았습니다.  그래서 지금은 수신만 블로킹이고 보내는 것은 Overlapped I/O입니다.   
  송수신할 때마다 스레드를 새로 만들지 않습니다.  컨텍스트를 스위칭하는 비용이 크고 스레드가 시작되는 타이밍이 들쑥날쑥하기 때문입니다.   
  블로킹이 풀린 후에야 소켓을 닫습니다.  소켓을 닫아 블로킹을 강제로 풀지 않습니다.   
( [*dlls/Game/scene/online/Online.cpp*](https://github.com/Mynsu/Sirtet_SFML/blob/master/dlls/Game/scene/online/Online.cpp) )

**#버퍼 복사 비용 줄이기**   
  대기열 서버는 소켓의 accept를 완료하면 그 소켓의 O/S 층 수신 버퍼를 없애 데이터를 직접 수신합니다.  이로써 O/S 층 버퍼와 애플리케이션 층 버퍼 사이에 복사가 일어나지 않아 비용을 아낄 수 있습니다.  대기열 서버는 버퍼가 필요할 만큼 무거운 작업을 하지 않기 때문에 가능한 일입니다.  대신 안정적으로 통신하도록 클라이언트에 있는 대기열 서버 소켓의 O/S 층 버퍼는 없애지 않았습니다.   
  메인 서버는 O/S 층 버퍼를 없애지 않았습니다.  메인 서버에 있는 수신 버퍼를 없앴다가 클라이언트가 송신한 데이터를 간혹 받지 못하는 현상을 목격했기 때문이었습니다.  또, 메인 서버에 있는 송신 버퍼를 없앴다가 클라이언트에 있는 수신 버퍼가 가득찼을 때 이상한 값을 수신한 것도 목격했기 때문이었습니다.   
( [*exes/QueueServer/QueueServer.cpp:177*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/exes/QueueServer/QueueServer.cpp#L177) )

**#전이중*full-duplex***   
  여러 개의 WSAOVERLAPPED 구조체를 두어 송수신을 동시에 할 수 있도록 구현했습니다.  한 개의 구조체로는 송신이나 수신을 완료한 후에야 다음 송신이나 수신이 가능하기 때문입니다.

**#스트림 헤더**   
  송수신하는 스트림의 머리에 ID(혹은 태그)와 헤더를 제외한 스트림의 길이가 붙습니다.  스트림의 꼬리는 표기하지 않습니다.   
( [_staticLibs/GameLib/Common.h:135_](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/staticLibs/GameLib/Common.h#L135) )

**#가짜 클라이언트 감지** **#해시 salt** **#키 스트레칭**   
  **#대기열 서버**에서 언급된 '초대장'은 현재 monotonic 시간과 salt와 키 스트레칭으로 얻은 해시 값입니다.  암복호화 해싱 알고리즘은 불필요하게 부하가 커 쓰지 않았습니다.   
( [*dlls/Game/scene/online/Waiting.cpp:17*](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/scene/online/Waiting.cpp#L17) )

**#클라이언트와 서버가 공유하는 모델 애셋*asset***   
  서버를 만들기 시작했을 땐 클라이언트가 쓰던 모델 애셋을 그대로 썼었습니다.  그러다 렌더링에 쓰이는 멤버 변수/함수는 서버에 불필요하므로 이를 뺀 서버용 모델 애셋을 따로 두었습니다.  하지만 따로 두는 건 개발 막바지에 했어야 했습니다.  같은 수정을 각각에 하다 보니 같아야 할 코드가 달라지는 등 실수가 나왔기 때문입니다.   
( [_dlls/Game/model/_](https://github.com/Mynsu/Sirtet_SFML/tree/master/dlls/Game/model)과 [*exes/Server/*](https://github.com/Mynsu/Sirtet_SFML/tree/master/exes/Server) 아래 *Tetrimino.h*와 *Stage.h* )

**#클라이언트와 서버의 동기화 방식**   
  게이머가 테트리미노를 움직이려고 입력하자마자 클라이언트가 먼저 입력을 적용해 테트리미노를 움직입니다.  동시에 클라이언트는 서버에 입력을 전달합니다.  이 때 서버가 입력을 검증*validation*한 다음 처리하고 결과를 클라이언트로 보내는 게 보통입니다.  하지만 이 게임에서는 허용되지 않은 방법으로 입력하더라도 오히려 손해만 보기 때문에 테트리미노를 움직이는 것은 검증하지 않습니다.
( [_dlls/Game/ui/PlayView.cpp:201_](https://github.com/Mynsu/Sirtet_SFML/blob/master/dlls/Game/ui/PlayView.cpp#L201) )   
  단, 테트리미노 낙하 속도와 관련한 입력은 검증합니다.  허용되지 않은 방법으로 속도를 낮추면 정당하지 않은 이득을 볼 수 있으므로 이 경우 클라이언트의 접속을 끊습니다.  클라이언트에서 선제적으로 검증하여 서버의 부담을 조금이나마 줄여줍니다.   
( [_dlls/Game/ui/PlayView.cpp:148_](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/dlls/Game/ui/PlayView.cpp#L148), [_exes/Server/Room.cpp:45_](https://github.com/Mynsu/Sirtet_SFML/blob/9a5e461624dd15256a1f22c75fd294aef88ab21d/exes/Server/Playing.cpp#L45) )

**#스트레스 테스트**   
  20년 3월 15일 기준, 클라이언트 100개까지 통과합니다.   

**#디버그 사례** **#디버그 모드에서 릴리즈 모드로** **#프로파일링**   
  개발 막바지에 디버그 모드에서 릴리즈 모드로 바꿔 빌드한 뒤 겪었던 이슈와 이를 해결한 과정을 소개합니다.  플레이 도중 클라이언트와 메인 서버의 통신이 끊기는 이슈였습니다.  빌드 모드를 바꿨더니 발생한 이슈이므로 먼저 비주얼 스튜디오의 프로젝트 설정부터 살펴봤습니다.  곧 메인 서버의 CRT가 /MDd에서 /MD로 바뀌면서 이슈가 발생한다는 것을 알아냈습니다.  메인 서버에서 디버그 모드의 코드 검사가 빠지면서 이슈가 발생했을 가능성이 크다고 추측했습니다.
  원인이 메인 서버에 있다는 것을 확인했으므로 이제 릴리즈 모드의 메인 서버가 어느 상태*state*일 때 이슈가 발생하는지 알아봤습니다.  이를 위해 상태별로 1 프레임 당 소요 시간을 관찰했습니다.  곧 방 하나에 클라이언트 둘 이상이 들어오자마자 1 프레임 당 300 밀리초를 넘기는 것을 발견했습니다.  그래서 방 접속 관련 로직을 집중적으로 프로파일링했고, 필요없는 WSAOVERLAPPED 구조체를 지우느라 병목이 걸린다는 것을 쉽게 알 수 있었습니다.  필요없는 구조체가 왜 이렇게 많이 만들어졌는지 알아내기 위해 코드를 분석했습니다.  그 결과, 구현 의도와 달리 이 구조체가 어떤 루프마다 만들어지고 있었다는 것을 찾아냈습니다.  이것이 이슈의 원인, 핵심 조건이었습니다.  이슈가 디버그 모드에서 잡히지 않은 것은 코드를 검사하기 때문이 아니라 느려서, 게임 전체 루프를 한 번 도는 데 소요되는 시간이 길어서였습니다.

**#OpenMP**   
  반복문에 OpenMP를 써봤는데 현저히 느려져 쓰지 않았습니다.

**#메소드 체이닝**   
  입출력 스트림, 문자열 스트림 외엔 쓰지 않습니다.

Readme.md를 마지막으로 수정한 날은
----------------------
2022년 2월 17일
