//
// Created by Daniil Bykov on 06.05.2025.
//
#include <iostream>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include <limits>
#include <iomanip>

int sign_bit = 1;
int exp_bits = 8;
int mant_bits = 23;

struct FP32 {
    std::array<bool, 1> sign;
    std::array<bool, 8> exp;
    std::array<bool, 23> mant;

    static FP32 from_float(float x) {
        FP32 fp;
        fp.sign[0] = x < 0;
        float abs_x = std::fabs(x);

        if (std::isnan(x)) {
            fp.exp.fill(1);
            fp.mant[0] = 1;
            for (int i = 1; i < exp_bits; ++i)
                fp.mant[i] = 0;
            return fp;
        }

        if (x == std::numeric_limits<float>::infinity() || x == -std::numeric_limits<float>::infinity()) {
            fp.exp.fill(1);
            fp.mant.fill(0);
            return fp;
        }
        if (abs_x == 0.0f) {
            fp.exp.fill(0);
            fp.mant.fill(0);
            return fp;
        }

        int int_part = (int)abs_x;
        float frac_part = abs_x - int_part;
        std::vector<bool> int_bin;
        if (int_part > 0) {
            while (int_part > 0) {
                int_bin.insert(int_bin.begin(), int_part % 2);
                int_part /= 2;
            }
        }

        std::vector<bool> frac_bin;
        int max_frac_bits = mant_bits;
        while (frac_bin.size() < max_frac_bits && frac_part > std::numeric_limits<float>::epsilon()) {
            frac_part *= 2;
            bool bit = frac_part >= 1.0f;
            frac_bin.push_back(bit);
            if (bit) frac_part -= 1.0f;
        }

        std::vector<bool> mantissa_bits;
        int exponent_unbiased;

        if (!int_bin.empty()) {
            exponent_unbiased = int_bin.size() - 1;
            // видаляємо першу 1, додаємо все інше + дробову
            for (size_t i = 1; i < int_bin.size(); ++i)
                mantissa_bits.push_back(int_bin[i]);
            for (bool bit : frac_bin)
                mantissa_bits.push_back(bit);
        } else {
            // ціла частина 0 — шукаємо першу 1 в дробовій
            int first_one = -1;
            for (size_t i = 0; i < frac_bin.size(); ++i) {
                if (frac_bin[i]) {
                    first_one = i;
                    break;
                }
            }
            if (first_one == -1) {
                fp.exp.fill(0);
                fp.mant.fill(0);
                return fp;
            }
            exponent_unbiased = -1 * (first_one + 1);
            for (size_t i = first_one + 1; i < frac_bin.size(); ++i)
                mantissa_bits.push_back(frac_bin[i]);
        }

        int exponent_biased = exponent_unbiased + 127;

        // денормалізоване число
        if (exponent_biased <= 0) {
            fp.exp.fill(0);
            int shift = 1 - exponent_biased;
            mantissa_bits.insert(mantissa_bits.begin(), shift, 0);
            for (int i = 0; i < mant_bits; ++i)
                fp.mant[i] = (i < mantissa_bits.size()) ? mantissa_bits[i] : 0;
            return fp;
        }

        // переповнення
        if (exponent_biased >= 255) {
            fp.exp.fill(1);
            fp.mant.fill(0);
            return fp;
        }

        // нормалізоване число
        for (int i = 7; i >= 0; --i)
            fp.exp[7 - i] = (exponent_biased >> i) & 1;
        for (int i = 0; i < mant_bits; ++i)
            fp.mant[i] = (i < mantissa_bits.size()) ? mantissa_bits[i] : 0;

        return fp;
    }

    float to_float() const {
        int e = 0;
        for (int i = 0; i < exp_bits; ++i) {
            e = (e << 1) | exp[i];
        }
        int m = 0;
        for (int i = 0; i < mant_bits; ++i) {
            m = (m << 1) | mant[i];
        }
        float frac = (e == 0) ? (m / pow(2, mant_bits)) : (1.0f + m / pow(2, mant_bits));
        int real_exp = e - 127;
        float val = std::ldexp(frac, real_exp);
        return sign[0] ? -val : val;
    }

    void print_bits() const {
        std::cout << "FP32 bits: ";
        std::cout << sign[0];
        for (bool b : exp) std::cout << b;
        for (bool b : mant) std::cout << b;
        std::cout << std::endl;
    }
};

void run_tests() {
    std::vector<float> test_vals = {
        0.0f,
        -0.0f,
        1.0f,
        -1.0f,
        3.14159265f,
        -42.42f,
        0.33325f,
        1024.0f,
        5.9604645e-08f,
        1.0e-45f,
        1.17549421e-38f,
        1.17549435e-38f,
        3.4028235e+38f,
        70000.0f,
        -70000.0f,
        std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::signaling_NaN()
    };

    std::cout << "=== FP32 Test Results ===\n";
    for (float val : test_vals) {
        FP32 encoded = FP32::from_float(val);
        float decoded = encoded.to_float();

        std::cout << "Input float: " << val << "\n";
        encoded.print_bits();
        std::cout << "Decoded float: " << decoded << "\n";

        bool all_exp_ones = std::all_of(encoded.exp.begin(), encoded.exp.end(), [](bool b){ return b; });
        bool all_exp_zeros = std::all_of(encoded.exp.begin(), encoded.exp.end(), [](bool b){ return !b; });
        bool all_mant_zeros = std::all_of(encoded.mant.begin(), encoded.mant.end(), [](bool b){ return !b; });
        bool any_mant_one = std::any_of(encoded.mant.begin(), encoded.mant.end(), [](bool b){ return b; });

        if (all_exp_ones && any_mant_one) {
            if (encoded.mant[0])
                std::cout << "Type: Quiet NaN\n";
            else
                std::cout << "Type: Signaling NaN\n";
        } else if (all_exp_ones && all_mant_zeros) {
            std::cout << "Type: Infinity\n";
        } else if (all_exp_zeros && all_mant_zeros) {
            std::cout << "Type: Zero\n";
        } else if (all_exp_zeros && any_mant_one) {
            std::cout << "Type: Denormalized\n";
        } else {
            std::cout << "Type: Normalized\n";
        }

        float abs_err = std::fabs(val - decoded);
        std::cout << "Abs error: " << abs_err << "\n";
        std::cout << "------------------------\n";
    }
}

template <typename FPType, typename FloatType>
void run_shared_tests(const std::string& type_name) {
    std::vector<FloatType> test_vals = {
        0.0,
        -0.0,
        1.0,
        -1.0,
        123.456,
        -123.456,
        0.333333,
        1024.0,
        0.0009765625,
        3.14159265358979,
        -42.42,
        1.0e-10,
        1.0e+10,
        std::numeric_limits<FloatType>::infinity(),
        -std::numeric_limits<FloatType>::infinity(),
        std::numeric_limits<FloatType>::quiet_NaN(),
        std::numeric_limits<FloatType>::signaling_NaN()
    };

    std::cout << "=== " << type_name << " Test Results ===\n";
    for (FloatType val : test_vals) {
        FPType encoded;

        if constexpr (std::is_same<FloatType, float>::value) {
            encoded = FPType::from_float(val);
        } else if constexpr (std::is_same<FloatType, double>::value) {
            encoded = FPType::from_double(val);
        } else if constexpr (std::is_same<FloatType, long double>::value) {
            encoded = FPType::from_long_double(val);
        }

        FloatType decoded;
        if constexpr (std::is_same<FloatType, float>::value) {
            decoded = encoded.to_float();
        } else if constexpr (std::is_same<FloatType, double>::value) {
            decoded = encoded.to_double();
        } else if constexpr (std::is_same<FloatType, long double>::value) {
            decoded = encoded.to_long_double();
        }

        std::cout << "Input: " << std::setprecision(15) << val << "\n";
        encoded.print_bits();
        std::cout << "Decoded: " << std::setprecision(15) << decoded << "\n";

        if (std::isnan(val) && std::isnan(decoded)) {
            std::cout << "Result: PASS (NaN)\n";
        }
        else if (std::isinf(val) && std::isinf(decoded) && (val > 0) == (decoded > 0)) {
            std::cout << "Result: PASS (Infinity)\n";
        }
        else {
            FloatType abs_err = std::fabs(val - decoded);
            std::cout << "Abs error: " << abs_err << "\n";
        }
        std::cout << "------------------------\n";
    }
}

int main() {
    run_shared_tests<FP32, float>("FP32");
    return 0;
}
