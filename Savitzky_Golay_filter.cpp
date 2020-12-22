// This data type represents the file stream generally, and has the capabilities of both ofstream and ifstream 
// which means it can create files, write information to files, and read information from files.
#include <fstream>

// Header that defines the standard input/output stream objects
#include <iostream>

#include <Eigen/Dense>
#include <cstdlib>
#include <math.h>

using namespace Eigen;

using std::cerr;        // Standard output stream for errors
using std::cout;        // Standard output stream
using std::endl;        // Inserts a newline character into the output sequence os and flushes it
using std::ifstream;

// For simplicity, the data length is pre-defined here
#define DATA_LENGTH 2048

// Defined here so memory was allocated on the heap (overflow occured if done dynamically (i.e. on the stack))
double y_smooth[DATA_LENGTH] = {};


/// <summary>
/// Loads data from a text file into an Eigen Vector
/// </summary>
/// <returns>Returns a pointer to a double array containing the file data</returns>
double* load_data()
{
    static VectorXd y(DATA_LENGTH);

    // std::ios::in -> Open a file for reading
    std::ifstream ifile("sample_data.txt", std::ios::in);

    //check to see that the file was opened correctly:
    if (!ifile.is_open()) {
        std::cerr << "There was a problem opening the input file!\n";
        exit(1);//exit or do additional error checking
    }

    double num = 0.0;
    int count = 0;
    //keep storing values from the text file so long as data exists (returns -1 when there is no data)
    while (ifile >> num) {
        y(count) = num;
        count++;
    }
    return y.data();
}
/// <summary>
/// Prints data into a text file. Stored as [x   y] to graph
/// </summary>
/// <param name="y_smooth">Data saved to the text file</param>
void print_data(double* y_smooth)
{
    std::ofstream myfile;
    myfile.open("FINAL_DATA.txt");
    myfile.precision(10);

    int i;
    for (i = 0; i < DATA_LENGTH; i++) {
        myfile << i << "\t" << y_smooth[i] << std::endl;
    }
}
/// <summary>
/// Computes the filter coefficients using Savitzky–Golay filter
/// </summary>
/// <param name="n">Degree of the fitting polynomial</param>
/// <param name="m">Number of sample points</param>
/// <returns></returns>
double* comp_coeff(int n, int m) {

    MatrixXd A = MatrixXd(m, n + 1);

    // Note: Savitzky–Golay filter only works with odd data points. Round down if even points are given
    int start = floor(round(1 - m) / 2);
    int end = floor((m - 1) / 2);
    int i_start = abs(start);

    // Matrix A is a Vandermonde matrix
    int i, j;
    for (i = start; i <= end; i++) {
        for (j = 0; j <= n; j++) {
            A(i + i_start, j) = pow(i, j);
        }
    }
    cout << "Here is the matrix A:\n" << A << endl;

    // Need to solve; coefficient = (A^T * A) * A^-1
    // Note: solving the inverse numerically on a computer, for large matrices, is ill-defined
    MatrixXd temp = (A.transpose() * A);
    MatrixXd coeff = temp.inverse() * A.transpose();

    // The filter coefficients are the first row of the solved matrix
    // I don't really get what the other rows are for. Something to do with derivatives
    static VectorXd coeff_row = coeff.row(0);
    static double* cc = coeff_row.data();

    cout << "Here are the coefficients:\n" << coeff_row << endl;

    return cc;
}
/// <summary>
/// Applies the coeeficients to the data from the provided text file.
/// When dealing with the endpoints, the method of extending the data was used
/// </summary>
/// <param name="coeff">Filtering coefficients</param>
/// <param name="n">Degree of the fitting polynomial</param>
/// <param name="data">Data to be filtered</param>
/// <param name="m">Number of sample points used in the filtering process</param>
/// <returns>The filtered data</returns>
double* coeff_filter(double* coeff, int n, double* data, int m) {

    int offset = floor((m - 1) / 2);

    double* start_data = new double[offset];
    double* end_data = new double[offset];

    // Extend the start and end of the data. 
    // Does the following: y3,y2,[y1, ... ,yn], yn?1, yn?2.
    for (int i = 0; i < offset; i++) {
        start_data[offset - i - 1] = data[i + 1];
        end_data[i] = data[DATA_LENGTH - 1 - i - 1];
    }


    // This basically takes into account the extended endpoints and selects the data accordingly
    // I've tested it, it does work
    for (int i = 0; i < DATA_LENGTH; i++) {
        for (int j = 0; j < m; j++) {

            if (i + j < offset)
            {
                y_smooth[i] += coeff[j] * start_data[i + j];
                //             cout << "data:  \n" << start_data[i + j] << endl;
            }
            else if (i + j >= DATA_LENGTH + offset)
            {
                y_smooth[i] += coeff[j] * end_data[(i + j) - DATA_LENGTH - offset];
                //             cout << "data:  \n" << end_data[(i + j) - DATA_LENGTH - offset] << endl;
            }
            else
            {
                y_smooth[i] += coeff[j] * data[(i + j) - offset];
                //             cout << "data:  \n" << data[(i + j) - offset] << endl;
            }

            //         cout << "coeff:  \n" << coeff[j] << endl;
            //         cout << "y_smooth[i]:  \n" << y_smooth[i] << endl;
        }
    }
    return y_smooth;
}

/// <summary>
/// Given N and M, along with a text file with data points, apply the Savitzky–Golay filter to the data
/// </summary>
int main(void)
{
    int N = 4;          // degree of polynomial
    int M = 65;         // # of points to fit polynomial to

    double* coeff;
    double* data;
    double* y_smooth;

    data = load_data();
    coeff = comp_coeff(N, M);
    y_smooth = coeff_filter(coeff, N, data, M);
    print_data(y_smooth);
}
