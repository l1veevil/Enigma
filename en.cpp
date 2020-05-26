#include <stdio.h>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>
#include <iostream>
#include "en.h"
#include <fstream>

using namespace std;

int mRotors,                // Количество роторов, размещенных в машине
                            // (1-based: 1-4)
    mSteps;                // Фактическое количество шагов шифрования
                           // = 2*mRotors + 2 (plugboard) + 1 (reflector)
int RotPos[Nrotors];     // Ротор помещен в каждое положение
char window[Nrotors],     // Символы в окне
Iwindow[Nrotors];    // Начальные значения в 'window' (для сброса)
string RotWiring[Nrotors]; // Обмотки ротора
char RotNotch[Nrotors];   // Положения переключения Ротора
int RotNumber[Nrotors];  // Какой физический Ротор (t,1-8,b,g)
string reflector,            // Проводка рефлектора
plugboard;   // Провода в коммутационной панели
int ReflType;              // Используемый рефлектор
char step[Nsteps];        // Массив для хранения шагов шифрования

// Файлы и переменные для настройки и отчетности

#define Nline 255

fstream inputFile, outputFile, logFile;
char outLine[Nline];  // выходная линия

void InitEnigma() // Инициализация по умолчанию
{
    int i;
   
    mRotors = 3;
    mSteps = (mRotors << 1) + 3;
    plugboard = PLUGBOARD;
    for (i = 0; i <= mRotors; ++i) {
        RotWiring[i] = ROTOR[i];
        RotNotch[i] = NOTCH[i];
        RotNumber[i] = i;
        Iwindow[i] = window[i] = 'a';
    }
    reflector = REFLECTOR[1];
    ReflType = 1;
}

void TryUserSetup()
// Попытка инициализации из пользовательского файла
{
    inputFile.open("esetup", ios::in);
    if(inputFile.is_open())      
     {
        SetPlugboard();
        SetRotorsAndReflector();
        inputFile.close();        
    }
}

void ProcessFile( const char* inFname,
    const char* encFname,
    const char* logFname)
{
    if (OpenFiles(inFname,
        encFname,
        logFname)) {
        SetRotorPositions();
        ReportMachine();
        ProcessPlainText();
        CloseFiles();
    }
}

void reset()
{
    for (int i = 1; i <= mRotors; ++i)
        window[i] = Iwindow[i];
}

void main()
{
    InitEnigma();
    TryUserSetup();

    ProcessFile("plain", "encrypt", "elog");
    reset();
    ProcessFile("encrypt", "decrypt", "dlog");
}

void SetPlugboard()  // сделайте соединения на плате штепсельной вилки
{
    int i, n, x;
    char p1, p2, ch;
    string inLine;
    // Прочитайте строку, содержащую пары букв, соответствующие парам штекеров,
    // соединенных проводом. Предполагается, что длина линии будет четной.

    
    inputFile >> inLine;     
    inLine[inLine.length() - 1] = '\0';
    n = inLine.length();

    for (i = 0; i < n; i += 2) {
        p1 = inLine[i];
        p2 = inLine[i + 1];
        x = index(p1);
        if ((ch = plugboard[x]) != p1) { // занят? -> разъединить
            plugboard[index(ch)] = ch;
            plugboard[x] = p1;
        }
        plugboard[x] = p2;                 // подключить
        x = index(p2);
        if ((ch = plugboard[x]) != p2) { // занят? -> разъединить
            plugboard[index(ch)] = ch;
            plugboard[x] = p1;
        }
        plugboard[x] = p1;                //подключить
    }
}

// Инициализация из файла 'esetup' (step 2)

void SetRotorsAndReflector()
{
    int i, n, rotor, rotPos;
    char ch, ringPos;
    string inLine;
    // Читать фактическое количество роторов "mRotors", рассчитаете
    // количество шагов шифрования "mSteps" и чтение строк "mRotors,
    // каждый из них содержит три символа, обозначающих:
    //
    //   (a) rotor ID (1-8,b,g)
    //   (b) rotor position (1-mRotors), and
    //   (c) ring character (position)

    inputFile >> inLine;    
    mRotors = ChrToInt(inLine[0]);
    if (mRotors > 4)
        mRotors = 4;
    mSteps = (mRotors << 1) + 3;
    for (i = 1; i <= mRotors; ++i) {
        inputFile >> inLine;  
        ch = inLine[0];
        if (isdigit((int)ch))
            rotor = ChrToInt(ch);
        else {
            ch = tolower(ch);
            rotor = ch == 'b' ? 9
                : ch == 'g' ? 10 : 0;
        }
        rotPos = ChrToInt(inLine[1]);
        ringPos = inLine[2];
        Iwindow[rotPos] = window[rotPos] = ringPos;
        PlaceRotor(rotPos, rotor);
    }

    // Прочитайте строку, содержащую обозначение отражателя (t,b,c,B,C)

    inputFile >> inLine;  
    ch = inLine[0];
    switch (ch) {
    case 't': n = 0; break;      case 'b': n = 1; break;
    case 'c': n = 2; break;      case 'B': n = 3; break;
    case 'C': n = 4; break;       default: n = 0; break;
    }
    reflector = REFLECTOR[n];
    ReflType = i;
}

int OpenFiles(const char* inFname,
    const char* encFname,
    const char* logFname)
{
    inputFile.open(inFname, ios::in);     
    outputFile.open(encFname, ios::out);  
    logFile.open(logFname, ios::out);     
    return    inputFile.is_open() && outputFile.is_open() && logFile.is_open();   
}

void CloseFiles()
{
    inputFile.close();
    outputFile.close();
    logFile.close();
}

void ReportMachine()
{
    logFile << "Plugboard mappings:\n";  
    logFile << ROTOR[0] << "\n";         
    logFile << plugboard << "\n";        

    logFile << "\nRotor wirings:\n";                           
    logFile << "position rotor ring setting notch sequence\n"; 
    for (int i = mRotors; i >= 1; --i)
            logFile << i << RotNumber[i] << window[i] << RotNotch[i] << RotWiring[i];            
    logFile << "\nreflector" << ReflType << reflector << "\n";
    logFile << "\nrotors:\n";    
    ShowRotors();
}

void ShowRotors()
{
    int i, j, k;
    string Rwiring;

    for (i = mRotors; i >= 1; --i) {
        logFile << i << ": ";  
        Rwiring = RotWiring[i];
        k = RotPos[i];
        for (j = 0; j < k; ++j)
            logFile << Rwiring[j]; 
        logFile << "->"; 
        for (j = k; j < Nchars; ++j)
            logFile << Rwiring[j]; 
        logFile << "\n";           
    }
}
int index(char c)
{
    // c in alphabet

    int i = 0;

    while ((i < Nchars) && (c != alphabet[i]))
        ++i;
    return i;
}

int ChrToInt(char c)
{
    // '0' <= c <= '9'

    return (int)(c - '0');
}

void PlaceRotor(int position, int r)
// комплект проводов одного Ротора
{
    RotWiring[position] = ROTOR[r];
    RotNotch[position] = NOTCH[r];
    RotNumber[position] = r;
}

void SetRotorPositions()
{
    int i, j, k;
    string Rwiring;
    char ch;

    for (i = 1; i <= mRotors; ++i) {
        j = RotNumber[i];
        ch = window[j];
        Rwiring = RotWiring[j];
        k = 0;
        while (Rwiring[k] != ch)
            ++k;
        RotPos[j] = k;
    }
}

int mod(int n, int modulus)  // простая функция по модулю
{
    while (n >= modulus)
        n -= modulus;
    while (n < 0)
        n += modulus;
    return n;
}

void ProcessPlainText()
{
    int i, n;
    char c1, c2;
    string inLine;

    logFile << "\n\nEncryption\n";       
    while (!inputFile.eof()) {           
        getline(inputFile, inLine, '\n');
        n = inLine.length();
        inLine[n] = '\0';
        for (i = 0; i < n; ++i) {
            c1 = inLine[i];
            if (isupper((int)c1))
                c1 = tolower(c1);

            c2 = encrypt(c1);

            ShowRotors();
            ShowWindow();
            logFile << " " << c1;   
            ShowSteps();
            logFile << "\n";  
            outLine[i] = c2;
        }
        logFile << "\n";    
        outLine[i] = '\0';
        outputFile << outLine << (!inputFile.eof() ? "\n" : ""); 
    } cout << endl;
}

void ShowWindow()
{
    int i;

    for (i = mRotors; i >= 1; --i)
        logFile << window[i] << " ";      
    logFile << "  ";                     
}

void ShowSteps()
{
    int i;

    for (i = 0; i < mSteps; ++i)
        logFile << " -> " << step[i];     
}

char encrypt(char c)
{
    int i, r;

    turn();                                          // перемещение роторов
    i = 0;                                           // пройти через:
    step[i++] = plugboard[index(c)];           //    коммутационная панель
    for (r = 1; r <= mRotors; ++r)
        step[i++] = RtoLpath(step[i - 1], r);     //   путь справа налево
    step[i++] = reflector[index(step[i - 1])]; //    отражатель
    for (r = mRotors; r >= 1; --r)                 //    путь слева направо
        step[i++] = LtoRpath(step[i - 1], r);
    step[i] = plugboard[index(step[i - 1])];   //    коммутационная панель

    return step[i];
}

void turn()   // определите, какие роторы должны вращаться
{
    int doit[Nrotors], n;
    string r1 = RotWiring[1]; string r2 = RotWiring[2]; string r3;

    if (mRotors > 3)
        r3 = RotWiring[3];

    // вычислите ширину шага для каждого ротора
    doit[1] = 1;
    for (int i = 2; i <= mRotors; ++i)
        doit[i] = 0;
    if ((RotNotch[1] == r1[RotPos[1]])
        ||
        (RotNotch[2] == r2[RotPos[2]]))  // двойной шаг
        doit[2] = 1;
    if (RotNotch[2] == r2[RotPos[2]])
        doit[3] = 1;
    if (mRotors > 3) {
        if (RotNotch[3] == r3[RotPos[3]])
            doit[4] = 1;
    }

    // поворот роторов " одновременно"
    for (n = 1; n <= mRotors; ++n)
        TurnRot(n, doit[n]);
}

void TurnRot(int n, int width)   // Поверните Ротор "n" "width" раз
{                                  // обертывание вокруг, если это необходимо
    string r;

    if (width > 0) {
        RotPos[n] = mod(RotPos[n] + width, Nchars);
        r = RotWiring[n];
        window[n] = r[RotPos[n]];
    }
}

// Трансформация по пути справа налево через роторы

char RtoLpath(char c, int r)  // преобразование символа "с" Ротором "r"
{
    int n, offset, idx, ret;
    string CurRotor;

    CurRotor = RotWiring[r];
    n = index(c);
    offset = index(CurRotor[RotPos[r]]);
    idx = mod(n + offset, Nchars);
    ret = mod(index(CurRotor[idx]) - offset, Nchars);
    return alphabet[ret];
}

// Трансформация на пути слева направо через роторы

char LtoRpath(char c, int r) // преобразование символа "с" Ротором "r"
{
    int n, m, offset, idx, newchar;
    string CurRotor;

    CurRotor = RotWiring[r];
    n = index(c);
    offset = index(CurRotor[RotPos[r]]);
    newchar = alphabet[mod(n + offset, Nchars)];

    m = 0;
    while (m < Nchars && CurRotor[m] != newchar)
        ++m;
    idx = mod(m - offset, Nchars);
    return alphabet[idx];
}