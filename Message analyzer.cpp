
#include <iostream> 
#include <vector> 
#include <string> 
#include <sstream> 

#define device1 "ПРМ ""Коршун"" "
#define device2 "ПРД ""Дрофа"" "
#define ERR_BEGIN "Ошибка начала сообщения"
#define ERR_END "Ошибка конца сообщения"
#define ERR_PARAM "Ошибка кода параметра"
#define ERR_VALUE "Ошибка в значении"
#define ERR_EMPTY "Пустое сообщение"
#define ERR_NUM_PARAM "Повторяющиеся параметры"

using namespace std;


class Message
{
public:
	string err;
	int flag = 0;
	stringstream res;
	Message(const string& str) {
		if (!str.empty()) {
			flag = def_protocol(str);
			if (flag == 1) to_vec_dev1(str);
			else if (flag == 2) to_vec_dev2(str);
		}
		else err = ERR_EMPTY;
	}

	int decoding()
	{
		if (flag == 1) return decoding_dev1();
		else if (flag == 2) return decoding_dev2();
		if (flag == 0) return 1;
	}

	string & get_error() {
		return err;
	}

	void get_decodMess(string & str) {
		str = name + '\n' + res.str();
	}

private:
	string name;
	vector<int> dec_mess;
	int def_protocol(const string&);
	void to_vec_dev1(const string&);
	void to_vec_dev2(const string&);
	int decoding_dev1();
	int decoding_dev2();
};

int  Message::decoding_dev2()
{
	if (dec_mess.empty())
		return 1;
	int F = dec_mess[0];
	int M = dec_mess[1];
	int P = dec_mess[2];
	if (F < 5000 || F > 50000 || (M != 1 && M != 2 && M != 10) || P > 100 || P % 10 != 0)
	{
		err = ERR_VALUE;
		return 4;
	}
	else
	{
		res << "Частота = " << F << " Гц\n" << "Режим работы = ";
		if (M == 1) res << "A1\n";
		else if (M == 2) res << "A2\n";
		else res << "F5R\n";
		res << "Установленная мощность в % = " << P << " %\n";
	}
	return 0;
}

int Message::def_protocol(const string & str)
{
	int len = str.length();
	string::const_iterator it = str.end();
	switch (str[0] & str[1])
	{
	case '3'& 'A':
		if (*(it - 2) == '2' && *(it - 1) == '1')
		{
			name = device1;
			return 1;
		}
		else err = ERR_END;
		break;
	case 'F'& 'F':
		if (str[3] != 'F' || str[4] != 'F')
			err = ERR_BEGIN;
		else
			if (*(it - 1) == 'F' && *(it - 4) == 'F' && *(it - 2) == 'E' && *(it - 5) == 'E')
			{
				name = device2;
				return 2;
			}
			else err = ERR_END;
		break;
	default:
		err = ERR_BEGIN;
		break;
	}
	return 0;
}

void Message::to_vec_dev1(const string & Str)
{
	int k;
	stringstream ss;
	for (string::const_iterator it = Str.begin() + 3; it < Str.end() - 3; it = it + 3)
	{
		ss << *it << *(it + 1);
		ss >> hex >> k;
		ss.clear();
		dec_mess.push_back(k);
	}
	if (dec_mess.empty()) err = ERR_PARAM;
	if (dec_mess.size() == 1) err = ERR_VALUE;
}

void Message::to_vec_dev2(const string & Str)
{
	int k;
	stringstream ss;
	string::const_iterator it = Str.begin();
	if (Str.size() != 23)
		err = ERR_VALUE;
	else
	{
		ss << *(it + 6) << *(it + 7) << *(it + 9) << *(it + 10);
		ss >> hex >> k;
		ss.clear();
		dec_mess.push_back(k);
		it += 12;
		for (int i = 0; i < 2; i++, it += 3)
		{
			ss << *it << *(it + 1);
			ss >> hex >> k;
			ss.clear();
			dec_mess.push_back(k);
		}
	}
}

int Message::decoding_dev1()
{
	if (dec_mess.empty() || dec_mess.size() == 1)
		return 1;
	vector<int>::iterator it = dec_mess.begin();
	int S = 0;
	int fF = 0, fS = 0, fC = 0;
	while (it < dec_mess.end())
	{
		switch (*it)
		{
		case 70:
			if (fF != 0)
			{
				err = ERR_NUM_PARAM;
				res.str("");
				return 4;
			}
			it++;
			if (distance(it, dec_mess.end()) < 4)
			{
				err = ERR_VALUE;
				res.str("");
				return 4;
			}
			if (*it < 47 || *it > 54 || *it == 48 && *(it + 1) == 48 || *(it + 1) < 47 || *(it + 1) > 58 || *(it + 2) < 47 || *(it + 2) > 58 || *(it + 3) != 48)
			{
				err = ERR_VALUE;
				return 4;
			}

			res << "Частота = ";
			for (int i = 0; i < 4; i++, it++) {
				res << (char)(*it);
			}
			res << '\n';
			if ((it - 1) == dec_mess.end()) it--;
			fF = 1;
			break;

		case 83:
			if (fS != 0)
			{
				err = ERR_NUM_PARAM;
				res.str("");
				return 4;
			}
			it++;
			if (distance(it, dec_mess.end()) == 0)
			{
				err = ERR_VALUE;
				res.str("");
				return 4;
			}
			S = *it;
			if (S != 49 && S != 50 && S != 51)
			{
				err = ERR_VALUE;
				return 4;
			}

			res << "Чувствительность = ";
			if (S == 49) res << "минимальная (1)\n";
			else
				if (S == 50) res << "средняя (2)\n";
				else
					if (S == 51) res << "максимальная (3)\n";
			it += 1;
			fS = 1;
			break;

		case 67:
			if (fC != 0)
			{
				err = ERR_NUM_PARAM;
				res.str("");
				return 4;
			}
			it++;
			if (distance(it, dec_mess.end()) < 2)
			{
				err = ERR_VALUE;
				res.str("");
				return 4;
			}
			if (*it != 48 && *it != 49 || *it == 48 && *(it + 1) == 48)
			{
				err = ERR_VALUE;
				return 4;
			}

			res << "Рабочий канал = " << (char)* it << (char) * (it + 1) << '\n';
			it += 2;
			fS = 1;
			break;

		default:
			err = ERR_PARAM;
			return 3;
		}
	}
	return 0;
}


int main()
{
	setlocale(LC_ALL, "Russian");

	int a;
	string Str, Res;

	cout << "Введите сообщение в шестнадцатиричном виде по одному байту через пробел:" << endl;
	getline(cin, Str);
	Message f = Message(Str);
	if (f.decoding() == 0)
		f.get_decodMess(Res);
	else cout << f.get_error() << '\n';
	cout << Res;
	system("pause");
}
