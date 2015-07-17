import serial
import numpy
import csv
import time
ard = serial.Serial('/dev/ttyACM0',38400)
third_octave_centers = (4.*(2.**(1./3.))**numpy.arange(13))
third_octave_left = third_octave_centers/(2.**(1./6.))     
def get_outliers(m,stdevs,data,out_csv,hr, n_stds =2.):
    dif = numpy.abs(data - m)
    outliers = dif > n_stds*stdevs
    n = len(m)
    for i in range(n):
        bin = third_octave_centers[i]
        outs = data[:,i][outliers[:,i]]
        out_csv.writerow([hr]+[bin]+outs.tolist())

z_accel = []
num_samps = 512
samp_rate = 200
cur_hr = time.localtime().tm_hour

hr_samples = []
fn = 'vibrations{}.csv'.format(time.localtime().tm_hour)
fn_outs = 'outliers{}.csv'.format(time.localtime().tm_hour)
with open(fn_outs,'wb') as csvOF:
    out_writer = csv.writer(csvOF)
    out_writer.writerow(['hour','bin','outliers'])
    with open(fn,'wb') as csvF:
        writer = csv.writer(csvF)
        writer.writerow(['hour', 'n_samples']+third_octave_centers.tolist())
        while True:
            while(ard.inWaiting() == 0):
                pass
            arduinoString = ard.readline()
            if(arduinoString.find('N') != -1):
                print('ok')
                print(third_octave_centers)
                prev_counts = third_octave_centers
                ard.write('Y')
            elif(arduinoString != ''):
                z = float(arduinoString)
                z_accel.append(z)
                if len(z_accel) == num_samps:
                    sp = numpy.fft.fft(numpy.asarray(z_accel))
                    freq = numpy.fft.fftfreq(len(z_accel),1./200.)
                    sp_abs = abs(sp)
                    oct_counts = numpy.zeros(len(third_octave_centers))
                    oct_num = numpy.zeros(len(third_octave_centers))
                    n = len(sp_abs)
                    for i in numpy.arange(n):
                        sp_i = sp_abs[i]
                        f_i = freq[i]
                        if (f_i > third_octave_left[0]) & (f_i < third_octave_centers[-1]*2.**(1./6.)):
                            oct = third_octave_centers[third_octave_left<f_i][-1]
                            oct_counts += 20.*numpy.log10(sp_i) * (third_octave_centers == oct)
                            oct_num += third_octave_centers==oct
                    oct_counts = oct_counts/oct_num
                    if time.localtime().tm_hour == cur_hr:
                        hr_samples.append(oct_counts)
                        print("---------------------------")
                        print(oct_counts - prev_counts)
                        prev_counts = oct_counts
                    else:
                        hr_numpy = numpy.asarray(hr_samples)
                        means = numpy.mean(hr_numpy,0)
                        stds = numpy.std(hr_numpy,0)
                        writer.writerow([cur_hr, len(hr_samples)] + means.tolist())
                        writer.writerow([cur_hr, len(hr_samples)] + stds.tolist())
                        get_outliers(means,stds,hr_numpy,out_writer,cur_hr)
                        cur_hr = time.localtime().tm_hour
                        hr_samples = []
                    z_accel = []
                    ard.write('Y')
