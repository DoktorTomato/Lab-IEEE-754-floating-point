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
int exp_bits = 15;
int mant_bits = 64;

struct FP80 {
    std::array<bool, 1> sign;
    std::array<bool, 15> exp;
    std::array<bool, 64> mant;

    static FP80 from_long_double(long double x) {
        FP80 fp;
        fp.sign[0] = x < 0;
        long double abs_x = std::fabsl(x);

        if (std::isnan(x)) {
            fp.exp.fill(1);
            fp.mant.fill(0);
            fp.mant[0] = 1;
            return fp;
        }

        if (std::isinf(x)) {
            fp.exp.fill(1);
            fp.mant.fill(0);
            return fp;
        }

        if (abs_x == 0.0L) {
            fp.exp.fill(0);
            fp.mant.fill(0);
            return fp;
        }

        uint64_t int_part = static_cast<uint64_t>(abs_x);
        long double frac_part = abs_x - static_cast<long double>(int_part);

        std::vector<bool> int_bin;
        while (int_part > 0) {
            int_bin.insert(int_bin.begin(), int_part % 2);
            int_part /= 2;
        }

        std::vector<bool> frac_bin;
        int max_frac_bits = mant_bits * 2;
        while (frac_bin.size() < max_frac_bits && frac_part > 0.0L) {
            frac_part *= 2;
            bool bit = frac_part >= 1.0L;
            frac_bin.push_back(bit);
            if (bit) frac_part -= 1.0L;
        }

        std::vector<bool> mantissa_bits;
        int exponent_unbiased;

        if (!int_bin.empty()) {
            exponent_unbiased = static_cast<int>(int_bin.size()) - 1;
            for (size_t i = 1; i < int_bin.size(); ++i)
                mantissa_bits.push_back(int_bin[i]);
            for (bool b : frac_bin)
                mantissa_bits.push_back(b);
        } else {
            int first_one = -1;
            for (size_t i = 0; i < frac_bin.size(); ++i) {
                if (frac_bin[i]) {
                    first_one = static_cast<int>(i);
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

        int exponent_biased = exponent_unbiased + 16383;

        if (exponent_biased <= 0) {
            fp.exp.fill(0);
            int shift = 1 - exponent_biased;
            mantissa_bits.insert(mantissa_bits.begin(), shift, 0);
            for (int i = 0; i < mant_bits; ++i)
                fp.mant[i] = (i < mantissa_bits.size()) ? mantissa_bits[i] : 0;
            return fp;
        }

        if (exponent_biased >= 32767) {
            fp.exp.fill(1);
            fp.mant.fill(0);
            return fp;
        }

        for (int i = 0; i < 15; ++i)
            fp.exp[i] = (exponent_biased >> (14 - i)) & 1;

        for (int i = 0; i < mant_bits; ++i)
            fp.mant[i] = (i < mantissa_bits.size()) ? mantissa_bits[i] : 0;

        return fp;
    }

    long double to_long_double() const {
        int exp_val = 0;
        for (int i = 0; i < 15; ++i)
            exp_val = (exp_val << 1) | exp[i];

        bool is_zero_exp = (exp_val == 0);
        bool is_all_ones_exp = (exp_val == 0x7FFF);

        long double mantissa = 0.0L;
        long double power = 0.5L;
        for (int i = 0; i < mant_bits; ++i) {
            if (mant[i])
                mantissa += power;
            power /= 2.0L;
        }

        if (!is_zero_exp && !is_all_ones_exp)
            mantissa += 1.0L;

        int true_exp = is_zero_exp ? -16382 : (exp_val - 16383);
        long double result = mantissa * std::powl(2.0L, true_exp);
        return sign[0] ? -result : result;
    }

    void print_bits() const {
        std::cout << "FP80 bits: ";
        std::cout << sign[0];
        for (bool b : exp) std::cout << b;
        for (bool b : mant) std::cout << b;
        std::cout << std::endl;
    }
};

void run_tests() {
    std::vector<long double> test_vals = {
        0.0L,
        -0.0L,
        1.0L,
        -1.0L,
        3.141592653589793238462643383279502884L,
        -42.42L,
        0.33325L,
        1024.0L,
        5.960464477539063e-08L,
        1.0e-45L,
        1.17549421e-38L,
        1.17549435e-38L,
        3.4028235e+38L,
        70000.0L,
        -70000.0L,
        std::numeric_limits<long double>::infinity(),
        -std::numeric_limits<long double>::infinity(),
        std::numeric_limits<long double>::quiet_NaN(),
        std::numeric_limits<long double>::signaling_NaN()
    };

    std::cout << "=== FP80 Test Results ===\n";
    for (long double val : test_vals) {
        FP80 encoded = FP80::from_long_double(val);
        long double decoded = encoded.to_long_double();

        std::cout << "Input long double: " << val << "\n";
        encoded.print_bits();
        std::cout << "Decoded long double: " << decoded << "\n";

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

        long double abs_err = std::fabsl(val - decoded);
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
    run_shared_tests<FP80, long double>("FP80");
    return 0;
}