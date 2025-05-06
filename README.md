# Lab-IEEE-754-floating-point
--- 
Team: [Bykov Danylo](https://github.com/DanyaBykov), [Ivan Shevchuk](https://github.com/DoktorTomato), [Maksym Dzoban](https://github.com/MaxDzioban)<br>

### Prerequisites

- cmake
- g++

### Compilation

To compile this project:
1. in lab directory 'mkdir builld'
2. cd build
3. cmake ..
4. cmake --build .

### Usage

In directory build:
- ./fp16 - to run program that tests FP16 
- ./fp32 - to run program that tests FP32
- ./fp64 - to run program that tests FP64
- ./fp80 - to run program that tests FP80

### Results
First of all let's understand what is FP16, FP32, FP64 and FP80 and how do they work.
FP is a floating point number format that is used to approximately represent decimal float numbers in binary form.
Each format has similar structures that looks like this:
| Format | Total Bits | Sign | Exponent | Mantissa (Fraction)     |
|--------|------------|------|----------|--------------------------|
| FP16   | 16         | 1    | 5        | 10                       |
| FP32   | 32         | 1    | 8        | 23                       |
| FP64   | 64         | 1    | 11       | 52                       |
| FP80   | 80         | 1    | 15       | 64 (with extra bit)      |

Now we need to understand what is Sign, Exponent and Mantisa
**Sign**: Is always one bit and tells us is the number negative or positive
**Exponent**: Is a number 2^exponent-bias which is used to multiply mantisa to get the decimal number. Where bias is: 2^(exponent_bits-1) - 1.
**Mantisa**: Stores the main digits (precision) of the number. It always starts from 1. so we only save digits after coma.

Now to turn decimal into binary we have this formula:

$$
\text{Value} = (-1)^{\text{sign}} \times (1.\text{mantissa}) \times 2^{\text{exponent} - \text{bias}}
$$

So using this we can turn decimal into binary and back

Now let's talk about difference between these formats:

The main difference is preciosion, because as the larger the total bit size the better the precision will be but it will obviously take more space and computations. As can be seen in the table above the larger the format the more of Mantisa we can store which increases the precision. 

Also there is an intresting differnce in FP80 because it doesn't asume that the first bit of Mantisa is 1 and it explicitly includes first bit in mantisa.

After running all programs on the same tests we recieve this results:

# Tests

| Input                | FP16 Error         | FP32 Error           | FP64 Error            | FP80 Error |
|---------------------|--------------------|----------------------|------------------------|------------|
| 0                   | 0                  | 0                    | 0                      | 0          |
| -0                  | 0                  | 0                    | 0                      | 0          |
| 1                   | 0                  | 0                    | 0                      | 0          |
| -1                  | 0                  | 0                    | 0                      | 0          |
| 123.456             | 0.0185012817382812 | 0                    | 1.281738e-06           | 0          |
| -123.456            | 0.0185012817382812 | 0                    | 1.281738e-06           | 0          |
| 0.333333            | 8.10325e-05        | 8.94070e-08          | 1.43604e-08            | 0          |
| 1024                | 0                  | 0                    | 0                      | 0          |
| 0.0009765625        | 0                  | 0                    | 0                      | 0          |
| 3.14159265358979    | 0.0009677410125732 | 0                    | 8.74228e-08            | 0          |
| -42.42              | 0.0137481689453125 | 0                    | 1.83105e-06            | 0          |
| 1e-10               | 1e-10              | 1e-10                | 1e-10                  | 0          |
| 10000000000         | 9.99993e+09        | 7.85252e+09          | 7.85252e+09            | 0          |
| inf                 | inf                | inf                  | inf                    | nan        |
| -inf                | inf                | inf                  | inf                    | nan        |
| nan (decoded=98304) | nan                | nan                  | nan                    | nan        |
| nan (decoded=98304) | nan                | nan                  | nan                    | nan        |


As we can see the precision is getting better for each next FP
