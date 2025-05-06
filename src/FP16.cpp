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
        float abs_x = std::fabs(x);
        int bias = 15;

        // === Випадок: нуль ===
        if (abs_x == 0.0f) {
            for (auto& b : fp.exp) b = 0;
            for (auto& b : fp.mant) b = 0;
            return fp;
        }

        // Отримуємо експоненту і нормалізовану дробову частину
        int exp;
        float frac = std::frexp(abs_x, &exp);  // abs_x = frac * 2^exp, 0.5 <= frac < 1

        int exponent = exp + bias - 1;

        if (exponent <= 0) {
            // === Ненормалізоване число ===
            for (auto& b : fp.exp) b = 0;

            // Здвигаємо frac для збереження в мантису без прихованої 1
            int shift = 1 - exponent;
            float denorm_frac = std::ldexp(frac, shift);

            int mant = static_cast<int>(denorm_frac * 1024.0f + 0.5f);  // округлення
            for (int i = 9; i >= 0; --i) {
                fp.mant[i] = (mant >> i) & 1;
            }

        } else if (exponent >= 31) {
            // === Переповнення — нескінченність ===
            for (int i = 0; i < 5; ++i) fp.exp[i] = 1;
            for (int i = 0; i < 10; ++i) fp.mant[i] = 0;

        } else {
            // === Нормалізоване число ===
            for (int i = 4; i >= 0; --i)
                fp.exp[4 - i] = (exponent >> i) & 1;

            float norm_frac = frac * 2.0f - 1.0f; // видаляємо приховану 1
            int mant = static_cast<int>(norm_frac * 1024.0f + 0.5f);
            for (int i = 9; i >= 0; --i) {
                fp.mant[i] = (mant >> i) & 1;
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
    float input = 700000.0f;
    FP16 encoded = FP16::from_float(input);
    encoded.print_bits();
    float decoded = encoded.to_float();
    std::cout << "Decoded float: " << decoded << "\n";
    return 0;
}
