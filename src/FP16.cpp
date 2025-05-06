//
// Created by Daniil Bykov on 06.05.2025.
//
#include <iostream>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include <limits>

struct FP16 {
    std::array<bool, 1> sign;
    std::array<bool, 5> exp;
    std::array<bool, 10> mant;

    static FP16 from_float(float x) {
        FP16 fp;
        fp.sign[0] = x < 0;
        float abs_x = x < 0 ? -x : x;
        int integer_part = (int)abs_x;
        float float_part = abs_x - integer_part;
        int c = 0;
        std::vector<bool> bits;
        int exp_count = 0; // counts number of bits from the integer part
        if (integer_part == 0) {
            int shift = 0;
            while (float_part < 1.0f && float_part > std::numeric_limits<float>::epsilon() && shift < 15) {
                float_part *= 2;
                ++shift;
            }
            exp_count = -shift + 1;
        } else {
            std::vector<bool> int_bits;
            while (integer_part > 0) {
                if (c==0) {
                    c = 1;
                } else {
                    int_bits.push_back(integer_part % 2);
                }
                integer_part /= 2;
            }
            // std::reverse(int_bits.begin(), int_bits.end());
            exp_count = int_bits.size();
            for (bool bit : int_bits) {
            if (bits.size() == 15)
                break;
            bits.push_back(bit);
            }
        }

        while (bits.size() < 10 && float_part > std::numeric_limits<float>::epsilon()) {
            float_part *= 2;
            bool bit = (float_part >= 1.0f);
            bits.push_back(bit);
            if (bit) {
            float_part -= 1.0f;
            }
        }

        int bias = 15;
        int exponent = exp_count + bias;
        for (int i = 4; i >= 0; --i) {
            fp.exp[4 - i] = (exponent >> i) & 1;
        }
        fp.sign[0] = x < 0;
        for (int i = 0; i < 10; ++i) {
            if (i < bits.size()) {
                fp.mant[i] = bits[i];
            } else {
                fp.mant[i] = false;
            }
        }

        return fp;
    }

    // Повертає float із FP8
    float to_float() const {
        // Зчитуємо експоненту
        int e = 0;
        for (int i = 0; i < 5; ++i) {
            e = (e << 1) | exp[i];
        }
        // Зчитуємо мантису
        int m = 0;
        for (int i = 0; i < 10; ++i) {
            m = (m << 1) | mant[i];
        }
        float frac = (e == 0) ? (m / 1024.0f) : (1.0f + m / 1024.0f);  // 10 біт мантиси

        int real_exp = e - 15; // знімаємо bias

        float val = std::ldexp(frac, real_exp); // frac * 2^exp

        return sign[0] ? -val : val;
    }

    // Вивід бітів FP8
    void print_bits() const {
        std::cout << "FP16 bits: ";
        std::cout << sign[0];
        for (bool b : exp) std::cout << b;
        for (bool b : mant) std::cout << b;
        std::cout << std::endl;
    }
};

int main() {
    float input = 9.4f;
    FP16 encoded = FP16::from_float(input);
    encoded.print_bits();
    float decoded = encoded.to_float();
    std::cout << "Decoded float: " << decoded << "\n";
    return 0;
}

