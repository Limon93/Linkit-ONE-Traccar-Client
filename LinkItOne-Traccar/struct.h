
struct GPSPos {
	float latitude;
	char latitude_dir;
	float longitude;
	char longitude_dir;
	int hour;
	int minute;
	int second;
	int	num;
	int fix;
  int alt;
  int spd;
	}MyGPSPos;

struct FlagReg {
	bool fix3D;			// flag to indicate if fix is 3D (at least) or not
	}MyFlag;

enum FixQuality {
	Invalid,	// 0
	GPS,		// 1
	DGPS,		// 2
	PPS,		// 3
	RTK,		// 4 Real Time Kinematic
	FloatRTK,	// 5
	DR,			// 6 Dead Reckoning
	Manual,		// 7
	Simulation,	// 8
	Error		// 9
	}GPSfix;

