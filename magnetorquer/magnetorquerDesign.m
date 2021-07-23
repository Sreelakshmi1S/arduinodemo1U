clear
clc
close all

%NOTE: ALL IN SI
% What can we change
current = 0;
N = 0;

% Fixed stuff
length = 70e-3;
moment = 0.2;
voltage = 5;
RperLength = 0.155;
rodRadius = 5e-3;
muR = 1453;          %iron
maxCurrent = 0.438;

%Stuff
currentMatrix = zeros(401,1);
nMatrix = zeros(401,1);
powerMatrix = zeros(401,1);
Nd = (4*(log(length/rodRadius) - 1))/(((length^2)/(rodRadius^2))-(4*log(length/rodRadius)));
CSarea = pi*(rodRadius^2);
tempVar = 1 + ((muR - 1)/(1 + (Nd*(muR - 1))));

for N = 100:500
    current = moment/(N*CSarea*tempVar);
    currentMatrix(N-99,1) = current;
    nMatrix(N-99,1) = N;
    Rnet = RperLength*2*pi*rodRadius*N;
    voltage = current*Rnet;
    power = voltage*current;
    powerMatrix(N-99,1) = power;
end

%plot
fig1 = figure();
plot(nMatrix,currentMatrix);
xlabel('No of turns (N)');
ylabel('Current (A)');

fig2 = figure();
plot(nMatrix,powerMatrix,'g');
xlabel('No of turns  (N)');
ylabel('Power  (Watts)');