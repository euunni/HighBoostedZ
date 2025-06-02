import os

def parse_list(list_dir):
    input_files = []
    for list_file in os.listdir(list_dir):
        if list_file.endswith(".list"):
            with open(os.path.join(list_dir, list_file)) as f:
                lines = [line.strip() for line in f if line.strip()]
                input_files.extend(lines)
    return input_files

def split_list(lst, split_size):
    for i in range(0, len(lst), split_size):
        yield lst[i:i + split_size]

def create_jobs(era, sample, list_dir, files_per_job):
    base_dir = os.getcwd()
    output_base = f"/u/user/haeun/CMSAnalysis/HighBoostedZ/Validation/HighBoostedZ/output/250526/{era}/{sample}"
    submit_dir = os.path.join(output_base, "Sub")
    log_dir = os.path.join(output_base, "Log")

    os.makedirs(submit_dir, exist_ok=True)
    os.makedirs(log_dir, exist_ok=True)

    input_files = parse_list(list_dir)
    print(f"[INFO] Found {len(input_files)} input files for sample '{sample}'.")

    job_splits = list(split_list(input_files, files_per_job))

    for idx, split in enumerate(job_splits):
        jobname = f"{sample}_{idx}"

        list_path = os.path.join(submit_dir, f"{jobname}.list")
        with open(list_path, "w") as f_list:
            for filepath in split:
                f_list.write(f"{filepath}\n")

        sh_path = os.path.join(submit_dir, f"{jobname}.sh")
        with open(sh_path, "w") as sh:
            sh.write("#!/bin/bash\n")
            sh.write("source /u/user/haeun/CMSAnalysis/HighBoostedZ/Validation/HighBoostedZ/envset.sh\n")
            sh.write("cd /u/user/haeun/CMSAnalysis/HighBoostedZ/Validation/HighBoostedZ/install\n")
            sh.write(f"./bin/Analyzer {sample} {era} {idx}\n")

        os.chmod(sh_path, 0o755)

        sub_path = os.path.join(submit_dir, f"{jobname}.sub")
        with open(sub_path, "w") as sub:
            sub.write("Universe = vanilla\n")
            sub.write(f"Executable = {sh_path}\n")
            sub.write(f"Output = {log_dir}/{jobname}.out\n")
            sub.write(f"Error  = {log_dir}/{jobname}.err\n")
            sub.write(f"Log    = {log_dir}/{jobname}.log\n")
            sub.write("should_transfer_files = YES\n")
            sub.write("when_to_transfer_output = ON_EXIT\n")
            sub.write(f"transfer_input_files = ../input/config/{era}/config.json\n")
            sub.write("queue 1\n")

        os.system(f"condor_submit {sub_path}")
