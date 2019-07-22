__kernel void simple_comparison(__global const unsigned int *vetor1, __global const unsigned int *vetor2, __global unsigned int *vetor3) {
	__private const unsigned short int match1 = 256;
	__private const unsigned int match2 = 1024, arraysize = 262144;
	__private unsigned short int counter1 = 0, counter2 = 0, for3, for4;
	__private unsigned int for1, for2;
	for (for1 = get_global_id(0) * match1; for1 < arraysize; for1 += 65536) {
		for (for2 = 0; for2 < arraysize; for2 += match1) {
			for (for3 = 0; for3 < match1; for3++) {
				for (for4 = 0; for4 < match1; for4++) {
					if (vetor1[for1 + for3] == vetor2[for2 + for4]) {
						counter1++;
					}
				}
			}
			if (counter1 == match1) {
				counter2++;
				counter1 = 0;
			}
			else{
				printf ("u can use this c print function to debug");
			}
		}
		if (counter2 == match2) {
			if (for1 > 0){
				vetor3[for1 / 256] = 1;
				counter2 = 0;
			}
			else {
				vetor3[for1] = 1;
				counter2 = 0;
			}
		}
	}
}