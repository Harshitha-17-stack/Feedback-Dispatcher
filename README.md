# Feedback Dispatcher – Multilevel Feedback Queue Scheduling

## Overview

Feedback Dispatcher is an Operating Systems project that demonstrates process scheduling using a three-level Multilevel Feedback Queue (MLFQ).

The project consists of:
- A native C implementation that demonstrates Linux process creation and process control.
- A separate React-based visual simulator that helps users understand scheduling behaviour through an interactive interface.

**Note:** The C implementation and React simulator run independently. They are not connected through live integration.

## Scheduling Algorithm

The dispatcher uses three feedback queues:

- **RQ0:** Highest-priority queue.
- **RQ1:** Medium-priority queue.
- **RQ2:** Lowest-priority queue.

The scheduling quantum is 1 time unit.

New processes enter RQ0. The dispatcher selects a process from the highest-priority non-empty queue. If a process is unfinished and another process is waiting when its quantum expires, it is suspended and moved to a lower-priority queue, if possible. The lowest-priority queue uses Round Robin scheduling.

## Technologies Used

- C
- Linux / Ubuntu
- GCC
- Linux process management and signals
- React and JavaScript (separate visual simulator)

## Project Files

| File | Description |
|---|---|
| `feedback.c` | Main dispatcher implementation |
| `worker.c` | Worker process implementation |
| `feedback.txt` | Input workload |

## How to Run the C Implementation

### 1. Clone the repository

```bash
git clone YOUR_GITHUB_REPOSITORY_URL
cd Feedback-Dispatcher
```

### 2. Compile the programs

```bash
gcc -Wall -Wextra feedback.c -o hostd
gcc -Wall -Wextra worker.c -o worker
```

### 3. Execute the dispatcher

```bash
./hostd feedback.txt
```

The dispatcher reads the process information from the input file and starts scheduling.

## Linux Concepts Demonstrated

- Process creation using `fork()` and `exec()`
- Process suspension and resumption using `SIGTSTP` and `SIGCONT`
- Process termination using `SIGINT`
- Process identification using PIDs
- Multilevel Feedback Queue scheduling
- Queue management and process dispatching

## Implementation Note

The dispatcher uses a logical scheduling timer to simulate CPU burst consumption. The worker process waits for signals; it does not perform CPU-intensive work. The scheduling time is therefore simulated rather than measured from actual CPU execution.

## React Visual Simulator

The separate React application provides:
- Process configuration
- Scheduling controls
- Three feedback queue visualizations
- Gantt chart
- Process status and event log
- Waiting time and turnaround time metrics

The React simulator performs its own JavaScript-based simulation and does not control the native Linux processes.

## Future Enhancements

- Integrate the C dispatcher with the React interface through a backend API.
- Display actual dispatcher events in the browser.
- Add more scheduling algorithms for comparison.
- Improve process monitoring and performance analysis.

## Team Members

This project was developed collaboratively by a team of six as part of the Operating Systems course.

1. Manvitha
2. Rishitha
3. Jyothishmathi
4. Harshitha Reddy
5. Harshitha
6. Meghana 


