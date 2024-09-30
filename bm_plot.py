import json
import matplotlib.pyplot as plt
import os
import re


def load_benchmark_data(file_path):
    with open(file_path) as f:
        return json.load(f)


def extract_benchmarks(data):
    return {bm['name']: bm['real_time'] for bm in data['benchmarks']}


def sanitize_filename(name):
    return re.sub(r'[<>:"/\\|?*]', '_', name)


def plot_benchmark_ratios():
    postfixes = [2 ** i for i in range(10, 17)]  # Степени двойки от 1024 до 65536
    ratios = {}

    for postfix in postfixes:
        base_file = f'BM_results/benchmark_chunk_vector_{postfix}.json'
        v2_file = f'BM_results/benchmark_chunk_vector_v2_{postfix}.json'

        if os.path.exists(base_file) and os.path.exists(v2_file):
            base_data = load_benchmark_data(base_file)
            v2_data = load_benchmark_data(v2_file)

            base_benchmarks = extract_benchmarks(base_data)
            v2_benchmarks = extract_benchmarks(v2_data)

            for name in base_benchmarks.keys():
                if name in v2_benchmarks:
                    ratio = v2_benchmarks[name] / base_benchmarks[name]
                    if name not in ratios:
                        ratios[name] = []
                    ratios[name].append(ratio)
                else:
                    if name not in ratios:
                        ratios[name] = []
                    ratios[name].append(None)
        else:
            for name in base_benchmarks.keys():
                if name not in ratios:
                    ratios[name] = []
                ratios[name].append(None)

    letter = 'a'
    for name, values in ratios.items():
        plt.figure(figsize=(10, 5))
        plt.plot(postfixes, values, marker='o')
        plt.xlabel('Postfix')
        plt.ylabel('Ratio (different / 4096)')
        plt.title(f'Benchmark Ratios for {name}')
        plt.xscale('log', base=2)
        plt.xticks(postfixes, labels=[str(p) for p in postfixes])
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(f'BM_results/{letter}_benchmark_ratios.png')
        plt.close()
        letter = chr(ord(letter) + 1)


plot_benchmark_ratios()