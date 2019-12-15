import os
import csv
from glob import iglob


OUTPUT = f'Data\processed-data.csv'


files = iglob(f'Data\data-processing\m4_data\*.csv')

with open(OUTPUT, 'w+', newline='') as out_file:

    writer = csv.writer(out_file)

    header_written = False
    flight_time_found = False
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
                    if row[0] == "log time":
                        pass
                    else:
                        # skip until flight computer was turned on at 8am
                        if not flight_time_found:
                            if row[1] == "15":
                                flight_time_found = True
                            else:
                                continue

                        running_time += 3
                        if row[1] == "0" or row[1] == "23":
                            # skip rows with no gps fix or error -> will
                            # screw up graphs using clock time.
                            pass
                        else:
                            gps_time = f'{row[1]}:{row[2]}:{row[3]}'

                            row[0] = running_time
                            row[1] = gps_time
                            row[4] = "{0:.6f}".format(float(row[4]) / 100)
                            row[5] = "-{0:.6f}".format(float(row[5]) / 100)
                            row[-8] = "{0:.2f}".format(float(row[-8]) - 9.81)
                            del row[13]
                            del row[12]
                            del row[11]
                            del row[9]
                            del row[3]
                            del row[2]

                            writer.writerow(row[0:-1])
                else:
                    assert row[0] == "log time", "header not found"
                    del row[12]
                    del row[11]
                    del row[10]
                    del row[8]
                    del row[3]
                    del row[2]
                    row[1] = "gps time"
                    row.insert(4, "GPS altitude")
                    writer.writerow(row[0:-1])
                    header_written = True
