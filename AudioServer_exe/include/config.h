#pragma once
#pragma warning(disable:4996)
#ifndef CONFIG_H
#define CONFIG_H


#define CONFIGFILE "advConfig.ini"

void ReadServerConfig(char** _ip, unsigned short* _port, int* _maxClient, bool* _logger);
void ReadFFTConfig(int* fftAttack, int* fftDecay, int* normalizeSpeed, int* peakthr, int* fps, int* changeSpeed);


#endif // !CONFIG_H
