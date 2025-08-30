# philosofer-dinner

## Overview

**philosofer-dinner** is a university project that implements the classic mutual exclusion and resource management problem known as "The Dining Philosophers" in C. Each philosopher has access to two chopsticks, and the number of chopsticks matches the number of philosophers. Learn more about the problem here: [Problema dei filosofi a cena](https://it.wikipedia.org/wiki/Problema_dei_filosofi_a_cena).

## Features

- 98% C, 2% Shell scripting
- Simulates concurrent philosophers and resource sharing
- Detects and/or avoids deadlock and starvation based on configuration
- Includes a background controller for monitoring philosopher states
- Logs results to `filelog.txt`

## How to Run

Use the `run.sh` script with 4 arguments:

```
./run.sh <num_philosophers> <detect_deadlock> <avoid_deadlock> <detect_starvation>
```

- **num_philosophers**: Number of philosophers
- **detect_deadlock**: Set to `1` to enable deadlock detection
- **avoid_deadlock**: Set to `1` to enable deadlock avoidance
- **detect_starvation**: Set to `1` to enable starvation detection

## Components

- **parent**: Main program executing the dining philosophers scenario
- **controllore**: Background controller monitoring the philosophers
- **filelog.txt**: Log file created by the controller

## Author

Developed by Samuele Costantini, IT Student at UNIPG

## License

See LICENSE file for details.