from src.performancebenchmark import run_benchmark
import sys



if __name__ == "__main__":
    if(len(sys.argv) == 2):
        run_benchmark(sys.argv[1])
    else:
        print("Wrong number of arguments")