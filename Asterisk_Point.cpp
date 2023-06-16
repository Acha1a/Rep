#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

using namespace std;

struct matched { // структурка, чтобы возвращать и подстроку, и индекс её вхождения
    int index;
    string quote;
};


bool is_match(string str, string &pattern, const size_t minLength) { // вторая функция, которая определяет - есть совпадение или нету

    if (str.size() < minLength) // рассмотривать случай не зачем, если для строки меньше минимально возможной длины pattern
        return false;

    if (pattern[0] == '*') // вначале строки не может быть '*'
        throw std::exception("Error\n");;

    for (size_t i = 0, j = 0; i < str.length() || j < pattern.size(); ++i, ++j) {
        if (i > str.size() - 1 && j > pattern.size() - 1) // на случай, когда всё проверено, но цикл рискует иметь ещё одну итерацию
        { // но этот случай, вероятно, никогда не наступит
            //quotes.push_back(str);
            return true;
        } 

        if (j >= pattern.size() && i < str.length()) // цикл пытается идти по pattern, когда он уже вышел за его пределы 
            return false;   // и, при этом, строка ещё не целиком оценена. Нехорошая ситуация.

        if (pattern[j] == '\\' && pattern[j + 1] == '*' && str[i] == '*') { // когда один астериск экранирован
            j++;
            continue;
        }

        if (pattern[j] == '\\' && pattern[j + 1] == '.' && str[i] == '.') { // когда точка экранирована
            j++;
            continue;
        }   

        if (pattern[j + 1] == '*') {
            if (pattern[j] != '\\' && pattern[j + 2] == '*') // нельзя использовать комбинацию "**", следует экранировать одну звёздочку
                throw std::exception("Error\n");
                //return false;

            if (j + 2 == pattern.length()) // если ".*" в конце pattern, и проверка дошла до этой конструкции, то есть совпадение
            {
                //quotes.push_back(str);
                return true;
            }

            if (pattern[j] == '.') { // когда ".*"
                // int ind = pattern.find(".*", j + 2); // узнаём, есть ли в оставшейся части строки ещё ".*"
                // if (ind < 0) { // если ".*" больше нету

                int jj = j + 2;
                while (pattern[jj] != '.' && pattern[jj] != '*'
                    && pattern[jj] != '\n' && pattern[jj] != '\r'
                    && pattern[jj] != '\t' && pattern[jj] != '\v'
                    && pattern[jj] != '\f' && jj < pattern.size())
                    ++jj;   // чтобы узнать первый индекс, стоящий за гранью допустимой подстроки

                if (pattern[jj] == '*') // перед '*' стоит какой-то необязательный символ, нужно перейти на него
                    --jj;

                string subbb = pattern.substr(j + 2, jj - j - 2); // нашли подстрочку, которая обязательно должна быть в оставшейся части str 

                string end_of_pattern = pattern.substr(j + 2); // остаток pattern, в котором нужно будет найти число вхождений определённой подстрочки

                for (string::iterator it = end_of_pattern.begin(); it != end_of_pattern.end(); ++it) // 65-83 удаление ключевых слов языка регулярных выражений из остатка pattern,
                {                                                                           // чтобы спокойно работать наапрямую с символами, которые были указаны в pattern
                    /*if (*it == '.') {
                        if (*(it + 1) == '*') {
                            end_of_pattern.erase(std::find(it + 1, end_of_pattern.end(), *(it + 1)));
                        }
                        end_of_pattern.erase(std::find(it, end_of_pattern.end(), *(it)));
                    }*/
                    if (*it == '*') {
                        --it;
                        end_of_pattern.erase(std::find(it, end_of_pattern.end(), *(it)));
                        end_of_pattern.erase(std::find(it, end_of_pattern.end(), *(it)));
                    }
                    if (*it == '.') {
                        end_of_pattern.erase(std::find(it, end_of_pattern.end(), *(it)));
                    }
                    if (it == end_of_pattern.end())
                        break;
                }

                /*for (string::iterator it = end_of_pattern.begin(); it != end_of_pattern.end(); ++it)
                {
                    if (*it == '*') {
                        --it;
                        end_of_pattern.erase(std::find(it, end_of_pattern.end(), *(it)));
                        end_of_pattern.erase(std::find(it, end_of_pattern.end(), *(it)));
                    }
                    if (it == end_of_pattern.end())
                        break;
                }*/

                int n_ind(-1), jjj(0);
                size_t count_in_pattern(0), count_in_str(0);
                vector<int>str_count; // вектор для хранения индексов, с которых начинается допустимая подстрока
                do { // цикл для нахождения количества вхождений в остаток паттерна определённой подстрочки
                    n_ind = end_of_pattern.find(subbb, jjj);
                    if (n_ind >= 0) {
                        jjj = n_ind + 1;
                        count_in_pattern++;
                    }
                    else break;
                } while (n_ind >= 0);

                size_t ii = i;
                do { // цикл для нахождения количества вхождений в оставшейся части str определённой подстрочки
                    n_ind = (int)str.find(subbb, ii);
                    if (n_ind >= 0) {
                        ii = n_ind + 1;
                        count_in_str++;
                        str_count.push_back(n_ind);
                    }
                    else break;
                } while (n_ind);

                if (count_in_pattern > count_in_str) // если pattern требует таких подстрочек больше, чем имеет оставшаяся часть str,
                    return false; // то, конечно, несовпадение

                i = str_count[count_in_str - count_in_pattern]; // так получаем нужный индекс, при котором пропустяться лишние символы 
                                                                 // и не затронутся важные будущие символы 
                --i;
                ++j;
                continue;
            }


            if (pattern[j + 2] == pattern[j]) { // случай some_char*some_char
                size_t k = j + 2, count(0); // k - новый индекс для счёта количества одинаковых сиволов после '*', count - счётчик этих символов
                while (pattern[k++] == pattern[j + 2]) // считаем количество одинаковых символов после '*'
                    count++;
                j += 2; // переводим индекс на элемент после '*'
                while (count-- > 0 && str[i++] == pattern[j++]) { // пропускаем одинаковое количество одинаковых элементов после '*' в pattern и str 
                    if (str[i - 1] != pattern[j - 1]) // если где-то символы не совпали, то pattern не совпадает с str
                        return false;
                }
                --j;
                while (str[i + 1] == str[i]) // поскольку до '*' могло быть большое количество этого одинакового символа, то следует пропустить все вхождения этого символа подряд
                    ++i;
                continue; // продолжаем цикл for
            }
            if (str[i] != pattern[j]) { // нулевое вхождение some_char*
                --i;
                ++j;
                continue;
            }
            while (str[i] == pattern[j]) // пропускаем все вхождения в ряд этого одного символа
            {
                i++;
                if (i == str.length())
                    break;
            }
            i--; // в следующий итерации цикла i опять инкрементируется, нужно заранее это нивелировать
            j++; // переход на '*', потом будет переход на символ для проверки
            continue;
        }

        if (str[i] == pattern[j] || pattern[j] == '.') // совпадение символов. При '.' совпадение будет всегда
            continue;
        else return false; // несовпадение. Значит pattern не соответствует этой str, вернуть false
    }
    //quotes.push_back(str);
    return true; // если перебор и сравнения произошли, и не было выбросов false, то имеется полное совпадение str и pattern
};

vector<matched> is_match(string &str, string &pattern) { // начальная функция для нахождения подстрок в строке. Возвращает вектор со всеми совпавшими подстроками

    if (strstr(pattern.c_str(), "**")) // сразу проверяем на недопустимую конструкцию "**"
        throw std::exception("Ass\n");

    string Ftrim(pattern); // строка для работы с pattern, чтобы не портить pattern

    Ftrim.erase(std::remove_if(Ftrim.begin(), // удаление '*' и '.' из pattern
        Ftrim.end(),
        [](unsigned char x) { return x == '.' || x == '*'; }),
        Ftrim.end());



    //for (string::iterator it = Ftrim.begin(); it != Ftrim.end(); ++it) // удаление '*' и '.' из pattern
    //{
    //    if (*it == '*') {
    //        --it;
    //        Ftrim.erase(std::find(it, Ftrim.end(), *(it)));
    //        Ftrim.erase(std::find(it, Ftrim.end(), *(it)));
    //    }
    //    if (*it == '.') {
    //        Ftrim.erase(std::find(it, Ftrim.end(), *(it)));
    //    }
    //    if (it == Ftrim.end())
    //        break;
    //}

    //for (string::iterator it = Ftrim.begin(); it != Ftrim.end(); ++it) // удаление '.' из pattern
    //{
    //    if (*it == '.') {
    //        Ftrim.erase(std::find(it, Ftrim.end(), *(it)));
    //    }
    //    if (it == Ftrim.end())
    //        break;
    //}

    const unsigned int minLength = Ftrim.length(); // у полученного "сырого" pattern берём длину


    //delete &Ftrim;

    vector<matched> quotes; // вектор, содержащий все удовлетворяющие pattern подстроки в str
    
    if (minLength > str.length()) // минимальный размер pattern не может быть больше длины str
        return quotes;   // возврат пустого вектора

    for (int i = 0; i < str.length()-minLength+1; ++i) { // двойной цикл для прохождения по строке с перебором всех возможных подстрок, которые могут совпасть с pattern
        for (int j = str.length() - 1; j>(i+minLength-2); --j) {
            if (is_match(str.substr(i, j - i+1), pattern, minLength))
                quotes.push_back({ i,str.substr(i, j - i + 1) });
        }
    
    }
    
    return quotes;
    
};

int main()
{
    string str, pattern;
    cout << "Where we find?\n";
    getline(cin, str);
    cout << "With which pattern?\n";
    getline(cin, pattern);
    //_strlwr_s((char*)str.c_str(),str.length()+1); // вариант приведения строки к нижнему регистру
    std::transform(str.begin(),  // приведение строки к нижнему регистру
        str.end(),
        str.begin(),
        [](unsigned char const& c) {
        return ::tolower(c);
    });
    //cout << "Result: " << is_match(str, pattern) << endl;
    
    vector<matched> result = is_match(str,pattern);

    for (auto it = result.begin(); it != result.end(); it++)
        cout << it->quote << "Found at " << it->index << endl;

    return 0;
}



