import os
import csv
from glob import iglob


OUTPUT = f'Data\combined-data-raw.csv'


files = iglob(f'Data\data-processing\m4_data\*.csv')

with open(OUTPUT, 'w+', newline='') as out_file:

    writer = csv.writer(out_file)

    header_written = False
    last_file = None
    running_time = 0

    for file in files:

        if last_file is not None:
            assert file > last_file, f"files out of order: {last_file}, {file}"
        last_file = file

        with open(file, 'r') as in_file:

            reader = csv.reader(in_file)
            for row in reader:
                if header_written:
                    if row[0] == "log time":    # skip header rows
                        pass
                    else:
                        gps_time = f'{row[1]}:{row[2]}:{row[3]}'

                        row[1] = gps_time
                        del row[3]
                        del row[2]

                        writer.writerow(row)
                else:
                    assert row[0] == "log time", "header not found"
                    del row[3]
                    del row[2]
                    row[1] = "gps time"
                    row.insert(4, "GPS altitude")
                    writer.writerow(row)
                    header_written = True
