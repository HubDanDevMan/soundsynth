import sndmath as s

nechlsong = (
"1.0 e4  1.0 c4 1.0g4 1.0d4",
"1.0 f#4 1.0 e4 1.0b4 1.0f#4",
"1.0 a4  1.0 g5 1.0d5 1.0a4")


s.create_melodies(("",""), "nechl.wav", s.SIN);
s.create_melodies(nechlsong, "nechl.wav", s.SIN);
s.create_melodies(nechlsong, "nechl.wav", s.SIN);
s.create_melodies(nechlsong, "nechl.wav", s.SIN);
s.create_melodies(nechlsong, "nechl.wav", s.SIN);
