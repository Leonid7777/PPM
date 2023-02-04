#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <unordered_map>
#include <string>
#include <array>

enum
{
    AGRESS = 80,
    MAX = 10000000,
    LENGTH_OF_CONTECST = 4,
    VER_OF_LAST = 257
};

void
change_the_borders(int64_t *l, int64_t *h, int64_t c, std::array<u_int32_t, 259> mas[])
{
    int64_t old_l = (*l);
    (*l) += (*mas)[c] * ((*h) - (*l) + 1) / (*mas)[258];
    (*h) = old_l + (*mas)[c + 1] * ((*h) - old_l + 1) / (*mas)[258] - 1;
}

void
change_of_mas(std::array<u_int32_t, 259> mas[], int64_t c)
{
    for(int64_t i = c + 1; i < 259; i++){
        (*mas)[i] += AGRESS;
    }
    while((*mas)[c + 1] - (*mas)[c] > MAX){
        u_int32_t old = (*mas)[0];
        for(int32_t i = 1; i < 259; i++){
            u_int32_t newl = (*mas)[i] - old + 1;
            old = (*mas)[i];
            (*mas)[i] = (*mas)[i - 1] + newl / 2;
        }
    }
}

void
new_write(int32_t out, int64_t *index, u_int8_t *val){
    write(out, val, sizeof(int8_t));
    (*index) = 8;
    (*val) = 0;
}


void
bits_plus_follow(u_int8_t bit, int32_t out, u_int8_t *val, int64_t *count_byte, int64_t *bits_to_follow)
{
    if(!(*count_byte)){
        new_write(out, count_byte, val);
    }
    if(bit){
        (*val) |= (1 << ((*count_byte) - 1));
    }
    (*count_byte) -= 1;
    while((*bits_to_follow))
    {
        if (!(*count_byte)) {
            new_write(out, count_byte, val);
        }
        if (!bit) {
            (*val) |= (1 << ((*count_byte) - 1));
        }
        (*count_byte) -= 1;
        (*bits_to_follow) -= 1;
    }
}

void
write_h_and_l(int64_t *l, int64_t *h, int32_t ouf, u_int8_t *byte, int64_t *count_byte, int64_t *bits_to_follow, int64_t Half, int64_t First_qtr, int64_t Third_qtr)
{
    for(;;) {
        if((*h) < Half){
            bits_plus_follow(0, ouf, byte, count_byte, bits_to_follow);
        } else if((*l) >= Half) {
            bits_plus_follow(1, ouf, byte, count_byte, bits_to_follow);
            (*l) -= Half;
            (*h) -= Half;
        }
        else if(((*l) >= First_qtr) && ((*h) < Third_qtr)){
            (*bits_to_follow)++;
            (*l) -= First_qtr;
            (*h) -= First_qtr;
        } else {
            break;
        }
        (*l) *= 2;
        (*h) += (*h) + 1;
    }
}

void
change_of_str(std::string *str, u_int8_t c)
{
    u_int64_t len = (*str).size();
    std::string new_str = "";
    new_str.push_back(c);
    new_str.insert(1, (*str), 0, len);
    (*str).clear();
    (*str).insert(0, new_str, 0, LENGTH_OF_CONTECST);
}

void
write_rev(int64_t *l, int64_t *h, int32_t inf, int64_t *value, u_int8_t *tmp, int64_t *index, int64_t Half, int64_t First_qtr, int64_t Third_qtr)
{
    for(;;){
         if ((*h) < Half) {
             ;
         } else if((*l) >= Half) {
            (*value) -= Half;
            (*l) -= Half;
            (*h) -= Half;
        }
        else if(((*l) >= First_qtr) && ((*h) < Third_qtr)){
            (*value) -= First_qtr;
            (*l) -= First_qtr;
            (*h) -= First_qtr;
       } else {
            break;
        }
        (*l) *= 2;
        (*h) += (*h) + 1;
        (*value) *= 2;
        (*value) |= ((*tmp) >> 7);
        (*tmp) <<= 1;
        (*index) -= 1;
        if(!(*index)){
            (*index) = 8;
            read(inf, tmp, sizeof(int8_t));
        }
    }
}

void
compress_ppm(char *ifile, char *ofile)
{
    int32_t inf = open(ifile, O_RDONLY, 0777);
    int32_t ouf = open(ofile, O_WRONLY | O_TRUNC | O_CREAT, 0777);
    int64_t First_qtr = 1073741824, Half = 2147483648, Third_qtr = 3221225472, count_byte = 8, bits_to_follow = 0, l = 0, h = 4294967295, c = 0;
    std::array<u_int32_t, 259> b;
    u_int8_t byte = 0;
    std::string str = "", compare = "";
    std::unordered_map <std::string, std::array<u_int32_t, 259> > mp;
    for(int32_t i = 0; i < 259; i++){
        b[i] = i;
    }
    while(read(inf, &c , sizeof(int8_t))){
        if(str.size() == 0){
            change_the_borders(&l, &h, c, &b);
            change_of_mas(&b, c);
            write_h_and_l(&l, &h, ouf, &byte, &count_byte, &bits_to_follow, Half, First_qtr, Third_qtr);
            change_of_str(&str, c);
        } else {
            for(u_int64_t i = str.size(); i >= 0; i--){
                compare.clear();
                compare.insert(0, str, 0, i);
                if(compare.size() == 0){
                    change_the_borders(&l, &h, c, &b);
                    change_of_mas(&b, c);
                    write_h_and_l(&l, &h, ouf, &byte, &count_byte, &bits_to_follow, Half, First_qtr, Third_qtr);
                    change_of_str(&str, c);
                    break;
                } else if(mp.count(compare) > 0){
                    if(mp[compare][c + 1] - mp[compare][c] == 0){
                        change_the_borders(&l, &h, 257, &mp[compare]);
                        write_h_and_l(&l, &h, ouf, &byte, &count_byte, &bits_to_follow, Half, First_qtr, Third_qtr);
                        change_of_mas(&mp[compare], c);
                        continue;
                    } else {
                        change_the_borders(&l, &h, c, &mp[compare]);
                        write_h_and_l(&l, &h, ouf, &byte, &count_byte, &bits_to_follow, Half, First_qtr, Third_qtr);
                        change_of_mas(&mp[compare], c);
                        change_of_str(&str, c);
                        break;
                    }
                } else {
                    std::array<u_int32_t, 259> next;
                    for(int k = 0; k < 259; k++){
                        next[k] = 0;
                    }
                    mp[compare] = next;
                    mp[compare][258] = VER_OF_LAST;
                    change_the_borders(&l, &h, 257, &mp[compare]);
                    write_h_and_l(&l, &h, ouf, &byte, &count_byte, &bits_to_follow, Half, First_qtr, Third_qtr);
                    change_of_mas(&mp[compare], c);
                    continue;
                }
            }
        }
    }
    c = 256;
    if(str.size() == 0){
        change_the_borders(&l, &h, c, &b);
        write_h_and_l(&l, &h, ouf, &byte, &count_byte, &bits_to_follow, Half, First_qtr, Third_qtr);
    } else {
        for(u_int64_t i = str.size(); i >= 0; i--){
            compare.clear();
            compare.insert(0, str, 0, i);
            if(compare.size() == 0){
                change_the_borders(&l, &h, c, &b);
                change_of_mas(&b, c);
                write_h_and_l(&l, &h, ouf, &byte, &count_byte, &bits_to_follow, Half, First_qtr, Third_qtr);
                change_of_str(&str, c);
                break;
            } else if(mp.count(compare) > 0){
                if(mp[compare][c + 1] - mp[compare][c] == 0){
                    change_the_borders(&l, &h, 257, &mp[compare]);
                    write_h_and_l(&l, &h, ouf, &byte, &count_byte, &bits_to_follow, Half, First_qtr, Third_qtr);
                    change_of_mas(&mp[compare], c);
                    continue;
                } else {
                    change_the_borders(&l, &h, c, &mp[compare]);
                    write_h_and_l(&l, &h, ouf, &byte, &count_byte, &bits_to_follow, Half, First_qtr, Third_qtr);
                    change_of_mas(&mp[compare], c);
                    change_of_str(&str, c);
                    break;
                }
            } else {
                std::array<u_int32_t, 259> next;
                for(int k = 0; k < 259; k++){
                    next[k] = 0;
                }
                mp[compare] = next;
                mp[compare][258] = VER_OF_LAST;
                change_the_borders(&l, &h, 257, &mp[compare]);
                write_h_and_l(&l, &h, ouf, &byte, &count_byte, &bits_to_follow, Half, First_qtr, Third_qtr);
                change_of_mas(&mp[compare], c);
                continue;
            }
        }
    }
    
    bits_to_follow += 1;
    if(l < First_qtr){
        bits_plus_follow(0, ouf, &byte, &count_byte, &bits_to_follow);
    } else {
        bits_plus_follow(1, ouf, &byte, &count_byte, &bits_to_follow);
    }
    write(ouf, &byte , sizeof(int8_t));
}



void
decompress_ppm(char *ifile, char *ofile){
    int32_t inf = open(ifile, O_RDONLY, 0777);
    int32_t ouf = open(ofile, O_WRONLY | O_TRUNC | O_CREAT, 0777);
    int64_t First_qtr = 1073741824, Half = 2147483648, Third_qtr = 3221225472, EOFC = 256, index = 0, c = 0, h = 4294967295, l = 0, value = 0;
    std::array<u_int32_t, 259> b;
    u_int8_t tmp = 0;
    lseek(inf, 0, SEEK_SET);
    std::unordered_map <std::string, std::array<u_int32_t, 259> > mp;
    std::string str = "", compare = "";
    for(int32_t i = 1; i < 259; i++){
        b[i] = i;
    }
    value = 0;
    for(int32_t i = 0; i < 4; i++){
        value <<= 8;
        read(inf, &tmp, sizeof(int8_t));
        value |= tmp;
    }
    for(;;){
        if(str.size() == 0){
            int64_t newl = ((value - l + 1) * b[258] - 1) / (h - l + 1);
            c = 0;
            while(b[c + 1] <= newl){
                c += 1;
            }
            if(c == EOFC){
                break;
            }
            change_the_borders(&l, &h, c, &b);
            change_of_mas(&b, c);
            change_of_str(&str, c);
            if(!index){
                index = 8;
                read(inf, &tmp, sizeof(int8_t));
            }
            write_rev(&l, &h, inf, &value, &tmp, &index, Half, First_qtr, Third_qtr);
        } else {
            for(u_int64_t i = str.size(); i >= 0; i--){
                compare.clear();
                compare.insert(0, str, 0, i);
                if(compare.size() == 0){
                    int64_t newl = ((value - l + 1) * b[258] - 1) / (h - l + 1);
                    c = 0;
                    while(b[c + 1] <= newl){
                        c += 1;
                    }
                    if(c == EOFC){
                        break;
                    }
                    change_the_borders(&l, &h, c, &b);
                    change_of_mas(&b, c);
                    if(!index){
                        index = 8;
                        read(inf, &tmp, sizeof(int8_t));
                    }
                    write_rev(&l, &h, inf, &value, &tmp, &index, Half, First_qtr, Third_qtr);
                    for(u_int64_t j = str.size(); j > 0; j--){
                        compare.clear();
                        compare.insert(0, str, 0, j);
                        change_of_mas(&mp[compare], c);
                    }
                    change_of_str(&str, c);
                    break;
                } else if(mp.count(compare) > 0){
                    int64_t newl = ((value - l + 1) * mp[compare][258] - 1) / (h - l + 1);
                    c = 0;
                    while(mp[compare][c + 1] <= newl){
                        c += 1;
                    }
                    if(c == 257){
                        change_the_borders(&l, &h, c, &mp[compare]);
                        if(!index){
                            index = 8;
                            read(inf, &tmp, sizeof(int8_t));
                        }
                        write_rev(&l, &h, inf, &value, &tmp, &index, Half, First_qtr, Third_qtr);
                        continue;
                    } else {
                        change_the_borders(&l, &h, c, &mp[compare]);
                        if(!index){
                            index = 8;
                            read(inf, &tmp, sizeof(int8_t));
                        }
                        write_rev(&l, &h, inf, &value, &tmp, &index, Half, First_qtr, Third_qtr);
                        std::string check = compare;
                        for(u_int64_t j = str.size(); j > 0; j--){
                            compare.clear();
                            compare.insert(0, str, 0, j);
                            change_of_mas(&mp[compare], c);
                            if(check == compare){
                                break;
                            }
                        }
                        change_of_str(&str, c);
                        break;
                    }
                } else {
                    std::array<u_int32_t, 259> next;
                    for(int k = 0; k < 259; k++){
                        next[k] = 0;
                    }
                    mp[compare] = next;
                    mp[compare][258] = VER_OF_LAST;
                    change_the_borders(&l, &h, 257, &mp[compare]);
                    if(!index){
                        index = 8;
                        read(inf, &tmp, sizeof(int8_t));
                    }
                    write_rev(&l, &h, inf, &value, &tmp, &index, Half, First_qtr, Third_qtr);
                    continue;
                }
            }
        }
        if(c == EOFC){
            break;
        }
        write(ouf, &c, sizeof(int8_t));
    }
}


int
main(int argc, char *argv[])
{
    compress_ppm(argv[1], argv[2]);
    decompress_ppm(argv[2], argv[3]);
    return 0;
}
