#include "InternalKeyMap.h"

#include "KeyboardInput.h"

extern const short InternalKeyMap::Forward[] =
{
	static_cast<short>(Key::Space), // 0
	static_cast<short>(Key::Apostrophe),
	static_cast<short>(Key::Comma),
	static_cast<short>(Key::Minus),
	static_cast<short>(Key::Period),
	static_cast<short>(Key::Slash),
	static_cast<short>(Key::N_0),
	static_cast<short>(Key::N_1),

	static_cast<short>(Key::N_2), // 8
	static_cast<short>(Key::N_3),
	static_cast<short>(Key::N_4),
	static_cast<short>(Key::N_5),
	static_cast<short>(Key::N_6),
	static_cast<short>(Key::N_7),
	static_cast<short>(Key::N_8),
	static_cast<short>(Key::N_9),

	static_cast<short>(Key::Semicolon), // 16
	static_cast<short>(Key::Equal),
	static_cast<short>(Key::A),
	static_cast<short>(Key::B),
	static_cast<short>(Key::C),
	static_cast<short>(Key::D),
	static_cast<short>(Key::E),
	static_cast<short>(Key::F),

	static_cast<short>(Key::G), // 24
	static_cast<short>(Key::H),
	static_cast<short>(Key::I),
	static_cast<short>(Key::J),
	static_cast<short>(Key::K),
	static_cast<short>(Key::L),
	static_cast<short>(Key::M),
	static_cast<short>(Key::N),

	static_cast<short>(Key::O), // 32
	static_cast<short>(Key::P),
	static_cast<short>(Key::Q),
	static_cast<short>(Key::R),
	static_cast<short>(Key::S),
	static_cast<short>(Key::T),
	static_cast<short>(Key::U),
	static_cast<short>(Key::V),

	static_cast<short>(Key::W), // 40
	static_cast<short>(Key::X),
	static_cast<short>(Key::Y),
	static_cast<short>(Key::Z),
	static_cast<short>(Key::LeftBracket),
	static_cast<short>(Key::Backslash),
	static_cast<short>(Key::RightBracket),
	static_cast<short>(Key::GraveAccent),

	static_cast<short>(Key::World_1), // 48
	static_cast<short>(Key::World_2),
	static_cast<short>(Key::Escape),
	static_cast<short>(Key::Enter),
	static_cast<short>(Key::Tab),
	static_cast<short>(Key::Backspace),
	static_cast<short>(Key::Insert),
	static_cast<short>(Key::Delete),

	static_cast<short>(Key::Right), // 56
	static_cast<short>(Key::Left),
	static_cast<short>(Key::Down),
	static_cast<short>(Key::Up),
	static_cast<short>(Key::PageUp),
	static_cast<short>(Key::PageDown),
	static_cast<short>(Key::Home),
	static_cast<short>(Key::End),

	static_cast<short>(Key::CapsLock), // 64
	static_cast<short>(Key::ScrollLock),
	static_cast<short>(Key::NumLock),
	static_cast<short>(Key::PrintScreen),
	static_cast<short>(Key::Pause),
	static_cast<short>(Key::F1),
	static_cast<short>(Key::F2),
	static_cast<short>(Key::F3),

	static_cast<short>(Key::F4), // 72
	static_cast<short>(Key::F5),
	static_cast<short>(Key::F6),
	static_cast<short>(Key::F7),
	static_cast<short>(Key::F8),
	static_cast<short>(Key::F9),
	static_cast<short>(Key::F10),
	static_cast<short>(Key::F11),

	static_cast<short>(Key::F12), // 80
	static_cast<short>(Key::F13),
	static_cast<short>(Key::F14),
	static_cast<short>(Key::F15),
	static_cast<short>(Key::F16),
	static_cast<short>(Key::F17),
	static_cast<short>(Key::F18),
	static_cast<short>(Key::F19),

	static_cast<short>(Key::F20), // 88
	static_cast<short>(Key::F21),
	static_cast<short>(Key::F22),
	static_cast<short>(Key::F23),
	static_cast<short>(Key::F24),
	static_cast<short>(Key::F25),
	static_cast<short>(Key::KP_0),
	static_cast<short>(Key::KP_1),

	static_cast<short>(Key::KP_2), // 96
	static_cast<short>(Key::KP_3),
	static_cast<short>(Key::KP_4),
	static_cast<short>(Key::KP_5),
	static_cast<short>(Key::KP_6),
	static_cast<short>(Key::KP_7),
	static_cast<short>(Key::KP_8),
	static_cast<short>(Key::KP_9),

	static_cast<short>(Key::KP_Decimal), // 104
	static_cast<short>(Key::KP_Divide),
	static_cast<short>(Key::KP_Multiply),
	static_cast<short>(Key::KP_Subtract),
	static_cast<short>(Key::KP_Add),
	static_cast<short>(Key::KP_Enter),
	static_cast<short>(Key::KP_Equal),
	static_cast<short>(Key::LeftShift),

	static_cast<short>(Key::LeftControl), // 112
	static_cast<short>(Key::LeftAlt),
	static_cast<short>(Key::LeftSuper),
	static_cast<short>(Key::RightShift),
	static_cast<short>(Key::RightControl),
	static_cast<short>(Key::RightAlt),
	static_cast<short>(Key::RightSuper),
	static_cast<short>(Key::Menu)
};

extern const short InternalKeyMap::Reverse[] =
{
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  // 0
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,

	0,    -1,   -1,   -1,   -1,   -1,   -1,   1,   // 32
	-1,   -1,   -1,   -1,   2,    3,    4,    5,
	6,    7,    8,    9,    10,   11,   12,   13,
	14,   15,   -1,   16,   -1,   17,   -1,   -1,

	-1,   18,   19,   20,   21,   22,   23,   24,  // 64
	25,   26,   27,   28,   29,   30,   31,   32,
	33,   34,   35,   36,   37,   38,   39,   40,
	41,   42,   43,   44,   45,   46,   -1,   -1,

	47,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  // 96
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,

	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  // 128
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,

	-1,   48,   49,   -1,   -1,   -1,   -1,   -1,  // 160
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,

	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  // 192
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,

	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  // 224
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,

	50,   51,   52,   53,   54,   55,   56,   57,  // 256
	58,   59,   60,   61,   62,   63,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	64,   65,   66,   67,   68,   -1,   -1,   -1,

	-1,   -1,   69,   70,   71,   72,   73,   74,  // 288
	75,   76,   77,   78,   79,   80,   81,   82,
	83,   84,   85,   86,   87,   88,   89,   90,
	91,   92,   93,   -1,   -1,   -1,   -1,   -1,

	94,   95,   96,   97,   98,   99,   100,  101, // 320
	102,  103,  104,  105,  106,  107,  108,  109,
	110,  -1,   -1,   -1,   111,  112,  113,  114,
	115,  116,  117,  118,  119
};
