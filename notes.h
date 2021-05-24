double frequencies[] = {
	262.0, 277.0, 294.0, 311.0,
	330.0, 349.0, 370.0, 392.0,
	415.0, 440.0, 466.0, 494.0,
	524.0 // C[+1]
};
char* names[] = {
	"C" , "C#", "D" , "D#",
	"E" , "F" , "F#", "G" ,
	"G#", "A" , "A#", "B"
};

// between B[-1] and C[0]
#define MIN_FREQ ( (247.0 + 262.0) / 2.0 )
// between B[0] and C[+1]
#define MAX_FREQ ( (494.0 + 524.0) / 2.0 )

void getNote(double frequency, char **name, int *octave, int *error) {
	*octave = 0;
	// constrain in reference octave
	while (frequency < MIN_FREQ) {
		frequency *= 2;
		(*octave)--;
	}
	while (frequency > MAX_FREQ) {
		frequency /= 2;
		(*octave)++;
	}
	// find first higher note
	int i = 1;
	while (i < 12 && frequency > frequencies[i]) {
		i++;
	}
	int middle = (frequencies[i]+frequencies[i-1])/2;
	if (frequency < middle) {
		i--;
		*error= (int)(100.0 * ( frequency-frequencies[i] ) / ( frequencies[i+1]-frequencies[i] ));
	} else {
		*error= (int)(100.0 * ( frequency-frequencies[i] ) / ( frequencies[i]-frequencies[i-1] ));
	}
	*name  = names[i];
}
