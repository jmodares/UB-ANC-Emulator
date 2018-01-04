#ifndef UBCONFIG_H
#define UBCONFIG_H

#define STL_PORT    5760
#define NET_PORT    15760
#define PWR_PORT    35760
#define PXY_PORT    45760

#define MAV_DIR "mav_"
#define OBJ_DIR "objects"

#define PACKET_END      "\r\r\n\n"
#define BROADCAST_ID    255

#define SERIAL_PORT "ttyACM0"
#define BAUD_RATE   115200

#define POINT_ZONE      1
#define TAKEOFF_ALT     5
#define GPS_ACCURACY    5

#define MISSION_TRACK_RATE  1000

#define SAVE_RATE   5

#endif // UBCONFIG_H
