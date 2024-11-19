% This defines the variables we need for simulink 
% Define the moments of inertia about each axis
Jx =J;
Jy = 1;
Jz = 1;

% Define the PID controller parameters
KP = 3;
KI = 1; 
KD = 1;

% Initial conditions
angVelInit = 100; % for some reason it drops to 1/2 the initial speed like immediately
ts = 100;

plot(sim('tumbling'))