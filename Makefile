
SYS: shuffle datashuffle navl encode decode 
	touch SYS


%.o: %.h
	cc -c -g  -DMI=$@.h decode.c -o $@

U=-Wunused-function -Wunused-variable

shuffle: shuffle.c
	cc -g $U shuffle.c -o shuffle
datashuffle: datashuffle.c
	cc -g $U datashuffle.c -o datashuffle
navl: navl.c HC.h
	cc -g $U navl.c -o navl
encode: encode.c clare.h
	cc -g $U encode.c -o encode
decode: decode.c clare.h
	cc -g $U decode.c -o decode
