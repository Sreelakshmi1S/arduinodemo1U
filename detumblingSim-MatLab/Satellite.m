function derivative = Satellite(t,state)

global BB invI I Bfieldmeasured pqrdotmeasured
global Bfieldnav pqrdotnav current voltage

%%Earth and sat params
Earth
Inertia

x = state(1);
y = state(2);
z = state(3);

q0123 = state(7:10);
pdot = state(11);
qdot = state(12);
rdot = state(13);
pqrdot = state(11:13);

%%translation
vel = state(4:6);
r = state(1:3);
rho = norm(r);
rhat = r/rho;
Fgrav = -(G*M*m/rho^2)*rhat;
acc = Fgrav/m;

%%Mag stuff
BI = [0;0;0];
BI(3) = 0.008;
BB = TIBquat(q0123)'*BI;

%%Bfield and ang velocty measured from snsr
[Bfieldmeasured,pqrdotmeasured] = Sensor(BB,pqrdot);

%%Navigation Block (correction)
[Bfieldnav,pqrdotnav] = Navigation(Bfieldmeasured,pqrdotmeasured);


%%CONTROL BLOCK
muB = Control(Bfieldnav,pqrdotnav);
Magtorquer_params
current = muB/(n*A*tempVar);
voltage = current*Rnet;

LMN_magtorquers = cross(muB,BB);

%%%Rotational Dynamics
PQRMAT = [0 -pdot -qdot -rdot;pdot 0 rdot -qdot;qdot -rdot 0 pdot;rdot qdot -pdot 0];
q0123dot = 0.5*PQRMAT*q0123;
H = I*pqrdot;
pqrddot = invI*(LMN_magtorquers - cross(pqrdot,H));

derivative = [vel;acc;q0123dot;pqrddot];
