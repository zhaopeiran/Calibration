%TO USE: in "interpolation.m":
%    - comment sections:
%       - "clear" and "close all";
%       - "ESTIMATION and CALIBRATION points"
%       - "USER and TEST dependant options"
%       - "PROGRAM OPTIONS"
%    - uncomment "function" line (first line)

clear();
close all;


%%%% INPUT DATA
program_options = struct( ...
    'method_interp'    , 1, ... %method for interpolation (1=polinomial, 2=griddata, 3=griddata (nearest))
    'method_pupil'     , 1, ... %use pupil as (1=rawpupil, 2=vectorpupil)
    'method_vergen'    , 2, ... %use pupil as (1=paper method, 2=raytracing, 3=fakedepth)
    'calc_error'       , 1, ... %calculate error (inter R; interp L; vergence)
    'calc_angle'       , 1, ... %calculate angle (inter; vergence; target)
    'calc_vergence'    , 1, ... %calculate vergence
    'paint_plot'       , 0, ... %show plots
    'paint_intp'       , 0, ... %show interpolation points
    'paint_vergence'   , 1, ... %show vergence points
    'paint_point'      , 0, ... %show point and lines to the eyes
    'paint_targetcalib', 0, ... %show target calibration points
    'paint_error'      , 1, ... %show error plot
    'point'            , 1 ...  %what point to paint
    );



person_data(1) = struct('name', 'Peiran12', 'interpupil', 62, 'dist_eyes_y', 410, 'eye_center_L', [72 72], 'eye_center_R', [176 61]);
person_data(2) = struct('name', 'Esteban2', 'interpupil', 57, 'dist_eyes_y', 405, 'eye_center_L', [86 76], 'eye_center_R', [155 89]);
person_data_use = 1;
% name         = name of the test subject for completing folder info
% interpupil   = inter pupil distance of the subject
% dist_eyes_y  = distance from subject eyes to origin ( Y axe )
% eye_center_L = position of the left Eye (X,Y)
% eye_center_R = position of the right Eye (X,Y)



% %test10
% folder_root = '/home/esguti/Dev/Eyetracking/Calibration/calibration_in/test10/';
% inputPoints(1,:) = [1:25];    % 290 mm from camera
% inputPoints(2,:) = [26:50];   % 390 mm from camera
% inputPoints(3,:) = [51:75];   % 490 mm from camera
% inputPoints(4,:) = [76:100];  % 590 mm from camera
% inputPoints(5,:) = [101:125]; % 690 mm from camera
% calibPoints(1,:) = [1:16];    % 290 mm from camera
% calibPoints(2,:) = [26:41];   % 390 mm from camera
% calibPoints(3,:) = [51:66];   % 490 mm from camera
% calibPoints(4,:) = [76:91];  % 590 mm from camera
% calibPoints(5,:) = [101:116]; % 690 mm from camera
% calib_plane = 5;

% %test11
% folder_root = '/home/esguti/Dev/Eyetracking/Calibration/calibration_in/test11/';
% inputPoints(1,:) = [1:25];    % 300 mm from camera
% inputPoints(2,:) = [26:50];   % 700 mm from camera
% calibPoints(1,:) = [1:16];    % 300 mm from camera
% calibPoints(2,:) = [26:41];   % 700 mm from camera
% calib_plane = 2;

% %test12
% folder_root = '/home/esguti/Dev/Eyetracking/Calibration/calibration_in/test12/';
% inputPoints(1,:) = [1:25];    % 205 mm from user
% inputPoints(2,:) = [26:50];   % 305 mm from user
% inputPoints(3,:) = [51:75];   % 405 mm from user
% calibPoints(1,:) = [1:25];    % 205 mm from user
% calibPoints(2,:) = [26:50];   % 305 mm from user
% calibPoints(3,:) = [51:75];   % 405 mm from user
% calib_plane = 3;

%test13
person_data(1) = struct('name', 'Peiran2' , 'interpupil', 62, 'dist_eyes_y', 410, 'eye_center_L', [72 72], 'eye_center_R', [176 61]);
person_data(2) = struct('name', 'Wolfgang', 'interpupil', 62, 'dist_eyes_y', 403, 'eye_center_L', [86 76], 'eye_center_R', [155 89]);
person_data(3) = struct('name', 'Thiago'  , 'interpupil', 54, 'dist_eyes_y', 400, 'eye_center_L', [86 76], 'eye_center_R', [155 89]);
person_data_use = 1;
folder_root = '/home/esguti/Dev/Eyetracking/Calibration/calibration_in/test13/';
inputPoints(1,:) = [1:25];    % 200 mm from user
inputPoints(2,:) = [26:50];   % 250 mm from user
inputPoints(3,:) = [51:75];   % 300 mm from user
inputPoints(4,:) = [76:100];  % 350 mm from user
inputPoints(5,:) = [101:125]; % 400 mm from user
calibPoints(1,:) = [1:16];    % 200 mm from user
calibPoints(2,:) = [26:41];   % 250 mm from user
calibPoints(3,:) = [51:66];   % 300 mm from user
calibPoints(4,:) = [76:91];   % 350 mm from user
calibPoints(5,:) = [101:116]; % 400 mm from user
calib_plane = 1;


%%%% PROGRAM
outputAngles = zeros(size(inputPoints,1),3);
errors = zeros(size(inputPoints,1),3);

for idx = 1:size(inputPoints,1)
    [ outputAngle, error ]= interpolation(inputPoints(idx,:), calibPoints(calib_plane,:), folder_root, person_data(person_data_use), program_options);
    outputAngles(idx,:) = median(outputAngle);
	errors(idx,:) = median(error);
end;

clearvars idx error outputAngle
%%%%