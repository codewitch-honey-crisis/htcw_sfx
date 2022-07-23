# SFX

SFX is a cross platform MIDI and audio library. It does produce output on its own, since doing so requires hardware specific calls. Drivers like htcw_i2s_audio use SFX to create sound.

Instead it provides foundation classes for manipulating audio and MIDI data.