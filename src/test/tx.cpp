// #include "../stream/iiostream-common.hpp" 

#include "../../ad9361/ad9361.hpp"

#include <vector>
#include <complex> 
#include <iostream> 

// iio_info -a  : show ip

int main (int argc, char **argv)
{   

    char *ip = argv[1];

	FILE *file, *file2;
	file  = fopen("/home/ivan/SDR_libiio_1.x/resurs_file/imag_part.txt", "r");
    file2 = fopen("/home/ivan/SDR_libiio_1.x/resurs_file/real_part.txt", "r");


	// complex *samples = (complex*) malloc(sizeof(complex)*5760);

	std::vector <comp> samples;
	double real, imag;

   	if(file)
    {
        for (int i = 0; i < 5760; i++){
            fscanf(file, "%lf\n", &imag);
            fscanf(file2, "%lf\n", &real);
			samples.push_back(comp(real,imag));
			// std::cout << "(real,imag) = " << real << ", " << imag << std::endl;
            // printf("%d ", (int)samples[i].imag);
            // printf("%d ", (int)samples[i].real);    
        }
    } 
	else
	{
		printf("error_file\n");
	}

	// std::cout << "samples1: " << samples[11].real() << ", " << samples[11].imag() << std::endl;
    // size_t tx_sample_sz;
    // struct iio_device *tx;
    // tx = initialize_device_tx(ip, tx_sample_sz, tx);
	
    // write_to_block(ip, samples, 3);
	// shutdown();
	//printf("* Starting IO streaming (press CTRL+C to cancel)\n");
	
	// tx(argc, argv);
	
	//shutdown();

	return 0;
}