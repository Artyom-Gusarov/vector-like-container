import os
import re

def update_readme_with_images():
    readme_path = '../README.md'
    image_dir = '.'  # Директория, где сохраняются изображения
    image_files = [f for f in os.listdir(image_dir) if f.endswith('_benchmark_ratios.png')]

    with open(readme_path, 'w') as readme_file:
        readme_file.write('# Benchmark Results\n\n## Benchmark Ratios\n\n')
        for image_file in image_files:
            benchmark_name = image_file.replace('_benchmark_ratios.png', '')
            readme_file.write(f'![Benchmark Ratios for {benchmark_name}](BM_results/{image_file})\n\n')

update_readme_with_images()