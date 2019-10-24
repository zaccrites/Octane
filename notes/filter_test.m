
% clear;
% close all;
% clc;

% % https://www.allaboutcircuits.com/technical-articles/design-of-fir-filters-design-octave-matlab/

% % We only have N = 3 (without tricks), so what does that get us?

% Adb = 40;


close all;
clear all;
clf;

pkg load signal;



% f1 = 10000;
% f2 = 15000;
% delta_f = f2-f1;
% Fs = 192000;
% dB  = 40;
% N = dB*Fs/(22*delta_f);

f1 = 1000;
f2 = 8000;
delta_f = f2-f1;
Fs = 44100;
dB  = -20 * log10(0.04)
N = dB*Fs/(22*delta_f)
% N = (8 * 3) - 1
% dB = N / Fs * 22*delta_f

f =  [f1 ]/(Fs/2);
hc = fir1(round(N)-1, f,'low');

% The above calculation produces a filter with these properties:
%  * 28 dB stopband rejection at 8 kHz (passband stops at 1 kHz)
%  * 7th-order, using 8 taps
%  * Linear phase?
% hc = fir1(N, f,'low');

% See also: https://octave.sourceforge.io/signal/function/fir1.html

% https://en.wikipedia.org/wiki/Digital_filter
% Get the transfer function
tf_den = 1;  % denominator is unity for FIR filters
Hz = tf(hc, 1, 1/Fs);
% Divide by largest power of z to make the filter causal
z = tf('z', 1/Fs);
Hz = Hz / z^7
% NOTE: Octave shows this as a fraction, but it means to just subtract 7 from
%  from the power of each z in the numerator, so they're all negative powers.

%      0.02005 z^7 + 0.06473 z^6 + 0.1664 z^5 + 0.2488 z^4 + 0.2488 z^3 + 0.1664 z^2 + 0.06473 z + 0.02005
% y1:  ---------------------------------------------------------------------------------------------------
%                                                      z^7
% =>
%
% y1: 0.02005 + 0.06473 z^-1 + 0.1664 z^-2 + 0.2488 z^-3 + 0.2488 z^-4 + 0.1664 z^-5 + 0.06473 z^-6 + 0.02005 z^-7

% Are FIR filters always symmetric? Do I only need half of the parameters?
% Apparently symmetrical-coefficient FIR filters have linear phase.

% https://tomroelandts.com/articles/why-use-symmetrical-fir-filters-with-an-odd-length
% https://dsp.stackexchange.com/questions/18413/why-the-number-of-filter-coefficients-in-fir-filter-has-to-be-an-odd-number
%
% I don't know if I care about having an odd number of taps.
% I'm not comparing the input and output, and I'd rather use all 8 DSPs
% unless I can't swing an 8th block RAM to store the 8th previous value.


% figure
plot((-0.5:1/4096:0.5-1/4096)*Fs,20*log10(abs(fftshift(fft(hc,4096)))))
axis([0 20000 -60 20])
title('Filter Frequency Response')
grid on
grid minor
