#
# ns-3 Network Library
#

NS3_VER = 3.27
#NS3_PRF = -debug
NS3_PRF = 

NS3_LIB = $$OUT_PWD/../../ns-3

DEPENDPATH += \
    $$NS3_LIB/lib \

INCLUDEPATH += \
    $$NS3_LIB/include/ns$$NS3_VER \

LIBS += \
    -lns$$NS3_VER-core$$NS3_PRF \
    -lns$$NS3_VER-network$$NS3_PRF \
    -lns$$NS3_VER-internet$$NS3_PRF \
    -lns$$NS3_VER-wifi$$NS3_PRF \
    -lns$$NS3_VER-mobility$$NS3_PRF \
    -lns$$NS3_VER-olsr$$NS3_PRF \
    -lns$$NS3_VER-aodv$$NS3_PRF \
    -L$$NS3_LIB/lib \
    -Wl,-rpath,$$NS3_LIB/lib \
