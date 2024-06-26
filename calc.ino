/*24.05.2024 Розанов Георгий Борисович
калькулятор для олимпиады

tg: https://t.me/Der_rozanov

копирование и публикация с разрешения автора

*комментарии к конкретным блокам кода расположены в плотную (без пустых строк) к блокам к которым 
они отностятся, расположение "в строке" "сверху" "снизу" значения не имеет. Висязие в воздухе комментарии
пояняют что происходит в программе безотносительно конкретных строк.

*/



//никаких библиотек))
#define char_dataPin  0   
#define char_clockPin  1  
#define char_latchPin  2 
#define seg_latchPin 3
#define clk 8
//дефайны пинов которые во время препроцессинга заменят номерами пинов имена

int numbers[10] = {
  0b01111110, 0b00001100, 0b10110110, 0b10011110, 0b11001100, 0b11011010, 0b11111010, 
  0b00001110, 0b11111110, 0b11011110
}; 
//набор двочиных кодов для отображения цифр 0-9 на 7 сегментном индикаторе

int seg[4] = {
  0b11111110, 0b11111101, 0b11111011, 0b11110111
};
//адреса каждого из 4ех разрядов индикатора

int r[] = {4,5,6,7};
//номера пинов для клавиатуры

const char symb [4][4] = {      // символы на клавиатуре
  {'1', '2', '3', '+'},
  {'4', '5', '6', '-'},
  {'7', '8', '9', 'C'},
  {'*', '0', '=', 'M'}
};
//набор симоволов согласно их расположению на клавиатуре 

int minus = 0b10000000;
//двоичный код символа "минус" для 7сегментного дисплея

char input_num = 0; //цифра которая в данный момент набирается 
int printed_num = 0; //число которое в данный момент отображается 
long unsigned int tim = millis(); //таймер организации опроса клавиатуры

int first_num = 0; //перове число при арифметике
int last_num = 0; //второе число при арифметике
bool act = 0; //флаг действия в зависимости от того что набрано на клавиатуре true - сложенени false - вычитание
bool neg = false; //флаг отрицательности набираемого числа, срабатывает когда перед набором числа нажат минус

/*функция отправки байта данных в сдвиговый регистр*/
void set595_reg(int8_t dataPin, int8_t clockPin, int8_t latchPin, int8_t data)
{
  digitalWrite(latchPin, LOW); //разрешение записи
  shiftOut(dataPin, clockPin, LSBFIRST, data); //запись
  digitalWrite(latchPin, HIGH); //запрет записи
}

/*функция отображения числа на индикаторе*/
void showNum(int16_t num)
{
  size_t lim = 3; //первично, число имеет лимит на показ 4ех цифр (считаем с нуля)

  /*однако если оно отрицательно*/
  if(num < 0)
  {
    lim = 2; //лимит показа уменьшается до 3ех цифр  
    num = abs(num); //число которое будем показывать берется по модулю (для корректного отображения)
    set595_reg(char_dataPin, char_clockPin, seg_latchPin, seg[3]); //выбирается 3й разряд (нумерация с нуля)
    set595_reg(char_dataPin, char_clockPin, char_latchPin, minus); //в 3й разряд прописывается минус
  }

  /*далее согласно лимитам*/
  for(int i=0;i<=lim;i++)
  {
    set595_reg(char_dataPin, char_clockPin, seg_latchPin, seg[i]); //выбирается текущий разряд
    set595_reg(char_dataPin, char_clockPin, char_latchPin, numbers[num%10]); //в него прописывается последняя цифра числа
    delay(10); //задерка для обеспечния динамической индикации
    num=num/10; //число делится на 10 (нацело) для того что бы последней цифрой стала цифра разрядом старше
    set595_reg(char_dataPin, char_clockPin, char_latchPin, 0b00000000); //очистка экрана для того что бы при индикации не было "ряби" цифр

    if(num == 0) break; //если число закончилось раньше лимита то незначащие нули показываться не будут
  }
}

void setup() 
{
  /*инициализация постоянных выходов устройства*/
  pinMode(char_dataPin, OUTPUT); //порт данных сдвиговых регистров
  pinMode(char_clockPin, OUTPUT); //порт синхронизации сдвиговых регистров
  pinMode(char_latchPin, OUTPUT); //порт разрешения записи регистра регистров (да, каламбур) 
  pinMode(seg_latchPin, OUTPUT); //порт разрешения записи регистра разрядов 
  pinMode(clk, OUTPUT); //порт синхронизации триггеров клавиатуры
}


void loop() 
{
  input_num = scantable(); //принимаем число с клавиатуры 

  /*если принятые числа оказались цифрами*/
  if(input_num> 47 && input_num<58)
  {
    if(neg) printed_num = printed_num * 10 - input_num+48;
    /*если ранее был выставлен флаг отрицательности то число умножается на 10 и вычитается новая цифра, 
    число "набирается" отриацательным
    вычитание 48 необходимо из-за особенностей таблицы ASCII*/

    else printed_num = printed_num * 10 + input_num-48;
    /*если ранее не был выставлен флаг отрицательности то число умножается на 10 и добавляется новая цифра, 
    число "набирается" положительным
    вычитание 48 необходимо из-за особенностей таблицы ASCII*/

    delay(100); //задержка что бы пользователь не ввел сразу несколько цифр
  }

  /*если никакое число еще не введено но нажат минус значит выставляется флаг отрицателности*/
  if(input_num == 45 && !printed_num) 
  {
    neg = true;
  }

  /*если какое-то число уже введено и нажата клавиша арифметического действия то*/
  if((input_num == 43 || input_num == 45) && printed_num )
  {
    /*в зависимости от клавиши ставится флаг действия*/
    if(input_num == 43) act = true;
    else act = false;

    /*первое число для арифметики приправнивается к набранному ранее*/
    first_num = printed_num;

    /*сбрасывается флаг отрицательности что бы следующее число было положительным, если не будет совершено
    действия для обратного*/
    neg = false;

    /*сбрасывается набранное число*/
    printed_num = 0;
    delay(100);//задержка что бы пользователь не ввел сразу несколько цифр
  }

  /*если нажата клавиша равно*/
  if(input_num == 61)
  {
    /*второе число для арифметрики приравнивается к набранному*/
    last_num = printed_num;

    /*в зависимости от флага действия происходит либо сложение либо вычитание*/
    if(act)
    first_num = first_num+last_num;
    else
    first_num = first_num-last_num;

    /*сбрасывается флаг отрицательности что бы следующее число было положительным, если не будет совершено
    действия для обратного*/
    neg = false;

    /*на экран выводится результат сохраненный в первом числе*/
    printed_num = first_num;
    delay(100);//задержка что бы пользователь не ввел сразу несколько цифр
  }

  /*если нажата клавиша очистки*/
  if(input_num == 67)
  {
    last_num = 0; //сбрасывается
    first_num = 0; //сбрасывается
    printed_num = 0; //сбрасывается
  }

  showNum(printed_num); //показывает текущее число на 7сегментном индикаторе
  delay(10); //задержка между итерациями основного цикла
}

/*функция сканирования матричной клавиатуры*/
char scantable()
{
  char res = 0; //инициализация результата по умолчанию. 
  // Прим! ноль здесь не цифра а номер символа означающий ничего ''.

  /*В цикле перебирается каждая строка клавиатуры отдельно*/
  for(int i = 0;i<4;i++) 
  {
    int vsp = scanrow(i); //скнирование одного ряда и сохранение результата в пределах строки 

    /*если результат не 9 (спецсимвол означающий отсутвие нажатых кнопок)*/
    if(vsp!=9) 
    { 
      res = symb[i][vsp]; //результат находится из матрицы по номеру строки i и стобца vsp
      break; //выход из цикла, так как результат уже найден, (два нажатия одновременно недопустимы)
    }
  }
  return res; //возврат результата
}

/*функция сканирования ряда*/
int scanrow(int k)
{
  int res = 9; //инициализация первичного результата, по умолчанию 9 - спецсимовл

  pinMode(r[k], OUTPUT); //установка текущей строки в режим выхода для установки высокого уровня 
  digitalWrite(r[k], HIGH); //установка высокго уровня на строке K

  /*тут регистрируется нажатие кнопки*/

  /*подаем полный импульс на двухтактный триггер, сохраняя результат в одной из защелок*/
  digitalWrite(clk, HIGH); 
  digitalWrite(clk, LOW);

  /*сбрасываем высокий уровень сигнала*/
  digitalWrite(r[k], LOW);

  /*Так как синхроимпульсов clk более не подается в защелках все еще лежат значения нажатых 
  или не нажатых кнопок*/

  /*все выходы переводятся в режимы ПОДЯТНУТЫХ(для избежания дребезга) входов */
  for(size_t i=0;i<4;i++)
  {
      pinMode(r[i], INPUT_PULLUP);
  }

  /*ищется вход куда пришел высокий уровень сигнала с защелок, хранящих состояние кнопок*/
  for(size_t i=0;i<4;i++)
  {
      if(digitalRead(r[i])) res = i; //при положительном результате номер столбца сохраняется в res
  }

  /*входы переводятся в режим выходов для последующих операций*/
  for(size_t i=0;i<4;i++)
  {
      pinMode(r[i], OUTPUT);
      digitalWrite(r[i], LOW); //установка нулевого значения на выходе (для избежания UB)
  }

  delay(10); //задержка между сканированием строк для обспечения отработки защелок и выходов

  return res; //возврат результата
}
