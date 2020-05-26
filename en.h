#include <stdio.h>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

#pragma once
#define Nchars 69  // Общее количество шифруемых символов
#define Mchars 70  // Размер буфера для строк, содержащих Nchars
#define Nrotors 11 // Максимальное количество роторов (1-based: 1-10)
#define Nrefls 5   // Общее количество отражателей (1-based: 1-4)
#define Nsteps 11  // Максимальное общее количество шагов шифрования

void SetRotorsAndReflector();
void SetPlugboard();
int OpenFiles(const char*, const char*, const char*);
void CloseFiles();
void ReportMachine();
void ShowRotors();
void SetRotorPositions();
void ProcessPlainText();
int index(char);
int ChrToInt(char);
void PlaceRotor(int, int);
char encrypt(char);
void ShowWindow();
void ShowSteps();
void turn();
char RtoLpath(char, int);
char LtoRpath(char, int);
void TurnRot(int, int);

string ROTOR[Nrotors] = {
    // входной алфавит ("rotor" 0, не используемый)
    "abcdefghijklmnopqrstuvwxyz0123456789.,:; ()[]'\"-+/*&~`!@#$%^_={}|\\<>?",
    // Ротор 1
    "ekmflgdqvzntowyhxuspaibrcj4.:5,63)-&;' +*7/\"](081[29?><\\|}{=^_%$#@!`~",
    // Ротор 2
    "ajdksiruxblhwtmcqgznpyfvoe093.]8[\"/1,7+':2)6&;(*5- 4?><\\|}{=^_%$#@!`~",
    // Ротор 3
    "bdfhjlcprtxvznyeiwgakmusqo13579,2(['/-&;*48+60.:\"]) ?><\\|}{=^_%$#@!`~",
    // Ротор 4
    "esovpzjayquirhxlnftgkdcmwb4] -(&90*)\"8[7/,;5'6.32:+1?><\\|}{=^_%$#@!`~",
    // Ротор 5
    "vzbrgityupsdnhlxawmjqofeck-&1[68'*\"(]3;7,/0+:9) 542.?><\\|}{=^_%$#@!`~",
    // Ротор 6
    "jpgvoumfyqbenhzrdkasxlictw9(6- \":5*)14;7&[3.0]/,82'+?><\\|}{=^_%$#@!`~",
    // Ротор 7
    "nzjhgrcxmyswboufaivlpekqdt;&976[2/:*]+1 \"508-,(4.)3'?><\\|}{=^_%$#@!`~",
    // Ротор 8
    "fkqhtlxocbjspdzramewniuygv5.)7',/ 219](3&[0:4+;8\"*6-?><\\|}{=^_%$#@!`~",
    // бета-Ротор
    "leyjvcnixwpbqmdrtakzgfuhos,4*9-2;8/+(1):3['0.&65\"7 ]?><\\|}{=^_%$#@!`~",
    // гамма-Ротор
    "fsokanuerhmbtiycwlqpzxvgjd5] .0;\"4[7:1'8*2+,)(&/-693?><\\|}{=^_%$#@!`~"
};


// Положение, в котором каждый Ротор заставляет своего левого соседа поворачиваться
// (Бета-и гамма-роторы могли использоваться только в четвертой позиции "Флотской Энигмы" и не имели эффекта нокдауна.
//  Таким образом, их положение в зазубринах бессмысленно)


char NOTCH[Nrotors] //Выемка
= { 'z', 'q', 'e', 'v', 'j', 'z', 'z', 'z', 'z', 'a', 'a' };

string REFLECTOR[Nrefls] = {
    // входной алфавит ("REFLECTOR" 0, не используемый)
    "abcdefghijklmnopqrstuvwxyz0123456789.,:; ()[]'\"-+/*&~`!@#$%^_={}|\\<>?",
    // reflector B, thick
    "yruhqsldpxngokmiebfzcwvjat*[\"7)],3(/;6 .:8415&2+-90'?<>\\|}{=_^%$#@`!~",
    // reflector C, thick
    "fvpjiaoyedrzxwgctkuqsbnmhl5-(980 *43[&/+62'.\")]1;:7,?<>\\|}{=_^%$#@`!~",
    // reflector B, dunn
    "enkqeuywjicopblmdxzvfthrgs4;.)0\"*+982 (1,:3/&-5'7[6]?<>\\|}{=_^%$#@`!~",
    // reflector C, dunn
    "rdobjntkvehmlfcwzrxgyipsuq[3 19;'.-47:,52+&0/6*8(]\")?<>\\|}{=_^%$#@`!~"
};

string PLUGBOARD = { //Коммутационная панель
    "abcdefghijklmnopqrstuvwxyz0123456789.,:; ()[]'\"-+/*&~`!@#$%^_={}|\\<>?"
};

const char* alphabet  // входной алфавит
=
"abcdefghijklmnopqrstuvwxyz0123456789.,:; ()[]'\"-+/*&~`!@#$%^_={}|\\<>?";