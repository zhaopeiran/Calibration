%function [angle, error] = interpolation(estimation_points, calib_points, folder_root, person_data, program_options)
%INTERPOLATION Estimate gaze points in 3D using interpolation.
%
%SYNOPSIS: [angle, error] = interpolation(estimation_points, calib_points, folder_root, person_data, program_options)
%
%INPUT:
%
%   - estimation_points: points to estimate
%
%   - calib_points: points to calibrate
%
%   - folder_root: root folder where the test is
%
%   - person_data: structure storing personal information of the subject
%     - name        : name of the test subject for completing folder info
%     - interpupil  : inter pupil distance of the subject
%     - dist_eyes_y : distance from subject eyes to origin ( Y axe )
%     - eye_center_L: position of the left Eye (X,Y)
%     - eye_center_R: position of the right Eye (X,Y)
%
%   - program_options = structure storing program options
%     - method_interp    : method for interpolation (1=polinomial, 2=griddata, 3=griddata (nearest))
%     - method_pupil     : use pupil as (1=rawpupil, 2=vectorpupil)
%     - method_vergen    : use pupil as (1=paper method, 2=raytracing, 3=fakedepth)
%     - calc_error       : calculate error (inter R; interp L; vergence)
%     - calc_angle       : calculate angle (inter; vergence; target)
%     - calc_vergence    : calculate vergence
%     - paint_plot       : show plots
%     - paint_intp       : show interpolation points
%     - paint_vergence   : show vergence points
%     - paint_point      : show point and lines to the eyes
%     - paint_targetcalib: show target calibration points
%     - paint_error      : show error plot
%     - point            : what point to paint
%
%OUTPUT:
%
%   - angle: angle in degrees between points and eyes (1=interpolation points, 2=estimated vergence point, 3=target point)
%
%   - error: error in degrees between real and estimated points (1=interpolation, 2=estimated vergence point)


% clear;
% close all;

% %%%%%% ESTIMATION and CALIBRATION points
% 200, 250, 300, 350, 400mm
% estimation_points = [1:25];       
 estimation_points = [26:50];
% estimation_points = [51:75];
% estimation_points = [76:100];
% estimation_points = [101:125];
% 
% calib_points = [1:16];
% calib_points = [26:41];
% calib_points = [51:66];
 calib_points = [76:91];
% calib_points = [101:116];
% %%%%%%

% %%%%%% USER and TEST dependant options
folder_root = '/Users/ZPR/Documents/Dev/input_data/test14/';
% person_data = struct('name', 'Peiran', 'interpupil', 62, 'dist_eyes_y', 410, ...
%                      'eye_center_L', [72 72], 'eye_center_R', [176 61]);
% person_data = struct('name', 'Esteban', 'interpupil', 57, 'dist_eyes_y',400,...
%                      'eye_center_L', [86 76], 'eye_center_R', [155 89]);
 person_data = struct('name', 'David', 'interpupil', 60, 'dist_eyes_y',400,...
                      'eye_center_L', [86 76], 'eye_center_R', [155 89]);
% person_data = struct('name', 'Wolfgang', 'interpupil', 62, 'dist_eyes_y',400,...
%                      'eye_center_L', [86 76], 'eye_center_R', [155 89]);
% person_data = struct('name', 'Thomas', 'interpupil', 62, 'dist_eyes_y',420,...
%                      'eye_center_L', [86 76], 'eye_center_R', [155 89]);
% person_data = struct('name', 'Thiago', 'interpupil', 62, 'dist_eyes_y',398,...
%                      'eye_center_L', [86 76], 'eye_center_R', [155 89]);
% %%%%%% PROGRAM OPTIONS
program_options = struct(...
    'method_interp'    , 1,...                         
    'method_pupil'     , 1,...
    'method_vergen'    , 1,...                         
    'calc_error'       , 1,...                       
    'calc_angle'       , 1,...
    'calc_vergence'    , 1,...
    'paint_plot'       , 1,...
    'paint_intp'       , 0,...
    'paint_vergence'   , 1,...
    'paint_point'      , 0,...
    'paint_targetcalib', 0,...
    'paint_error'      , 1,...
    'point'            , 1 ...
    );
% %%%%%%


%%%%%% FILES and FOLDERS
folder_in  = [folder_root person_data.name];
file_pupcalL  = [folder_in '/test_' person_data.name '_pupil_pos_L_calib.txt'];
file_pupcalR  = [folder_in '/test_' person_data.name '_pupil_pos_R_calib.txt'];
file_pupL     = [folder_in '/test_' person_data.name '_pupil_pos_L.txt'];
file_pupR     = [folder_in '/test_' person_data.name '_pupil_pos_R.txt'];
file_tar      = [folder_in '/test_' person_data.name '_target_pos.txt'];
file_tarcal   = [folder_in '/test_' person_data.name '_target_pos_calib.txt'];
%%%%%%

%%%%% ENVIRONMENT dependant options
dist_camera = 1200; % distance from subject to camera ( Z axe )
%%%%%


%%%%% SEVERAL values
eyesR = [person_data.interpupil/2 person_data.dist_eyes_y 0];
eyesL = [-person_data.interpupil/2 person_data.dist_eyes_y 0];

title_est = [num2str(estimation_points(1)) '-' num2str(estimation_points(size(estimation_points,2)))];
title_cal = [num2str(calib_points(1)) '-' num2str(calib_points(size(calib_points,2)))];
%%%%%%


%%%%%% EXTRACT DATA
% MpupcalL = dlmread(file_pupcalL,';',1,0);
MpupcalL = dlmread(file_pupL,';',1,0);
PupLeftPosCalX  = MpupcalL(:,2);
PupLeftPosCalY  = MpupcalL(:,3);
% MpupcalR = dlmread(file_pupcalR,';',1,0);
MpupcalR = dlmread(file_pupR,';',1,0);
PupRightPosCalX = MpupcalR(:,2);
PupRightPosCalY = MpupcalR(:,3);
MpupL = dlmread(file_pupL,';',1,0);
PupLeftPosX  = MpupL(:,2);
PupLeftPosY  = MpupL(:,3);
MpupR = dlmread(file_pupR,';',1,0);
PupRightPosX = MpupR(:,2);
PupRightPosY = MpupR(:,3);
Mtar = dlmread(file_tar,';',1,0);
TargetPosX = Mtar(:,2);
TargetPosY = Mtar(:,3);
TargetPosZ = Mtar(:,4);
% Mtarcal = dlmread(file_tarcal,';',1,0);
Mtarcal = dlmread(file_tar,';',1,0);
TargetPosCalXcal = Mtarcal(:,2);
TargetPosCalYcal = Mtarcal(:,3);
TargetPosCalZcal = Mtarcal(:,4);
%%%%%%

%%%%%% SELECT INPUT DATA
if( program_options.method_pupil == 2)
    PupLeftPosCalX  = (PupLeftPosCalX  - person_data.eye_center_L(1))./norm((PupLeftPosCalX  - person_data.eye_center_L(1)));
    PupLeftPosCalY  = (PupLeftPosCalY  - person_data.eye_center_L(2))./norm((PupLeftPosCalY  - person_data.eye_center_L(2)));
    PupRightPosCalX = (PupRightPosCalX - person_data.eye_center_R(1))./norm((PupRightPosCalX - person_data.eye_center_R(1)));
    PupRightPosCalY = (PupRightPosCalY - person_data.eye_center_R(2))./norm((PupRightPosCalY - person_data.eye_center_R(2)));
    PupLeftPosX     = (PupLeftPosX     - person_data.eye_center_L(1))./norm((PupLeftPosX     - person_data.eye_center_L(1)));
    PupLeftPosY     = (PupLeftPosY     - person_data.eye_center_L(2))./norm((PupLeftPosY     - person_data.eye_center_L(2)));
    PupRightPosX    = (PupRightPosX    - person_data.eye_center_R(1))./norm((PupRightPosX    - person_data.eye_center_R(1)));
    PupRightPosY    = (PupRightPosY    - person_data.eye_center_R(2))./norm((PupRightPosY    - person_data.eye_center_R(2)));
end;
TargetPosCalXcal = TargetPosCalXcal( calib_points );
TargetPosCalYcal = TargetPosCalYcal( calib_points );
TargetPosCalZcal = TargetPosCalZcal( calib_points );

XL = [PupLeftPosX, PupLeftPosY];
XR = [PupRightPosX, PupRightPosY];
Yt = [TargetPosX, TargetPosY, TargetPosZ];
XL = XL(estimation_points,:);
XR = XR(estimation_points,:);
Yt = Yt(estimation_points,:);

XLcal = [PupLeftPosCalX, PupLeftPosCalY];
XRcal = [PupRightPosCalX, PupRightPosCalY];
XLcal = XLcal( calib_points ,:);
XRcal = XRcal( calib_points ,:);
%%%%%%


%%%%%% INTERPOLATION
if( program_options.method_interp == 1)
    V = XLcal;
    P = [TargetPosCalXcal TargetPosCalYcal];
    % PV = @(b,V)  [[(b(1).*V(:,1)+b(2))] [(b(3).*V(:,2)+b(4))]]; % Objective Function: [b2 + b1*x] [b4 + b3*y]
    PV = @(b,V) [ ...
        (  b(6)*V(:,1).*V(:,2) +  b(5).*V(:,2).^2 +  b(4).*V(:,1).^2 + b(3).*V(:,2) + b(2).*V(:,1) + b(1) ) ...
        ( b(12)*V(:,2).*V(:,1) + b(11).*V(:,1).^2 + b(10).*V(:,2).^2 + b(9).*V(:,1) + b(8).*V(:,2) + b(7) )  ];
    [B, rn, r] = lsqcurvefit(PV, ones(12,1), V, P);
    Pc = PV(B,XL);
    plx = Pc(:,1);
    ply = Pc(:,2);
    
    V = XRcal;
    P = [TargetPosCalXcal TargetPosCalYcal];
    % PV = @(b,V)  [[(b(1).*V(:,1)+b(2))] [(b(3).*V(:,2)+b(4))]]; % Objective Function: [b2 + b1*x] [b4 + b3*y]
    PV = @(b,V) [ ...
        (  b(6)*V(:,1).*V(:,2) +  b(5).*V(:,2).^2 +  b(4).*V(:,1).^2 + b(3).*V(:,2) + b(2).*V(:,1) + b(1)) ...
        ( b(12)*V(:,2).*V(:,1) + b(11).*V(:,1).^2 + b(10).*V(:,2).^2 + b(9).*V(:,1) + b(8).*V(:,2) + b(7))  ];
    [B, rn, r] = lsqcurvefit(PV, ones(12,1), V, P);
    Pc = PV(B,XR);
    prx = Pc(:,1);
    pry = Pc(:,2);
elseif( program_options.method_interp == 2)
    plx = griddatan(XLcal,TargetPosCalXcal,XL);
    ply = griddatan(XLcal,TargetPosCalYcal,XL);
    
    prx = griddatan(XRcal,TargetPosCalXcal,XR);
    pry = griddatan(XRcal,TargetPosCalYcal,XR);
else
    plx = griddatan(XLcal,TargetPosCalXcal,XL,'nearest');
    ply = griddatan(XLcal,TargetPosCalYcal,XL,'nearest');
    
    prx = griddatan(XRcal,TargetPosCalXcal,XR,'nearest');
    pry = griddatan(XRcal,TargetPosCalYcal,XR,'nearest');
end;
plz = ones(numel(ply),1)* TargetPosCalZcal(1);
prz = ones(numel(pry),1)* TargetPosCalZcal(1);
%%%%%%



%%%%%% VERGENCE
if( program_options.calc_vergence == 1)
    pvx = zeros(size(plx,1),1); pvy = zeros(size(ply,1),1); pvz = zeros(size(plz,1),1);
    if( program_options.method_vergen == 1 )
        %paper method
        dist_plane = abs(eyesR(3) - TargetPosCalZcal(1));   % distance from subject to calibration plane ( Z axe )
        for idx = 1:numel(plx)
            deltaX = abs(plx(idx) - prx(idx));
            if(prx(idx) > plx(idx)) % after calibration plane
                pvz(idx) = person_data.interpupil * dist_plane / ( person_data.interpupil-deltaX);
                pvx(idx) = (( person_data.interpupil * prx(idx))-(deltaX * person_data.interpupil/2)) / (person_data.interpupil - deltaX);
                pvy(idx) = ((pry(idx)+ply(idx))/2 - person_data.dist_eyes_y) * pvz(idx) / dist_plane + person_data.dist_eyes_y;
            else
                if(prx(idx) == plx(idx)) % on fixed plane
                    pvx(idx) = prx(idx);
                    pvy(idx) = (pry(idx) + ply(idx)) / 2;
                    pvz(idx) = dist_plane;
                else % before calibration plane
                    pvz(idx) = person_data.interpupil * dist_plane / (person_data.interpupil+deltaX);
                    pvx(idx) = ((person_data.interpupil * prx(idx))+(deltaX * person_data.interpupil/2)) / (person_data.interpupil + deltaX);
                    pvy(idx) = ((pry(idx)+ply(idx))/2 - person_data.dist_eyes_y) * pvz(idx) / dist_plane + person_data.dist_eyes_y;
                end;
            end;
        end;
    elseif( program_options.method_vergen == 2 )
        %shortest line distance
        dist_short = zeros(size(plz,1),1); point_short_left = zeros(size(plx,1),3); point_short_right = zeros(size(prx,1),3);
        for idx = 1:numel(plx)
            l1 = [[eyesL(1) eyesL(2) eyesL(3)]; [plx(idx) ply(idx) plz(idx)]];
            l2 = [[eyesR(1) eyesR(2) eyesR(3)]; [prx(idx) pry(idx) prz(idx)]];
            [d Pc Qc]=distBW2lines(l1, l2);
            %[Pc Qc] = getLinesClosestPoints(l1(1,:), l1(2,:), l2(1,:), l2(2,:)); d = 0;
            dist_short(idx) = d;
            point_short_left(idx,:)  = Pc(:);
            point_short_right(idx,:) = Qc(:);
            pvx(idx) = Qc(1);
            pvy(idx) = Qc(2);
            pvz(idx) = Qc(3);
        end;
    else
        %fakedepth
        for idx = 1:numel(plx)
            pvx = median([prx, plx],2);
            pvy = median([pry, ply],2);
            pvz = median([prz, plz],2);
        end;
%         %transform
%         RealPoints = Yt(:,1:2);
%         ImagePoints = [pvx pvy];
%         T = fitgeotrans(ImagePoints,RealPoints,'projective');
%         RealMeasurePoint = tformfwd(T,ImagePoints);
    end;
end;

%%%%%%


%%%%%% ERROR CALCULATION
if( program_options.calc_error == 1)
    error_angle = zeros(size(plx,2),3);%%%%%%% error of angle
    error_dist = zeros(size(plx,2),1); %%%%%%% error of euclidean distance 
    for idx = 1:numel(plx)
        dotp = plx(idx) * Yt(idx,1) + ply(idx) * Yt(idx,2) + plz(idx) * Yt(idx,3);
        lenSq1 = plx(idx) * plx(idx) + ply(idx) * ply(idx) + plz(idx) * plz(idx);
        lenSq2 = Yt(idx,1) * Yt(idx,1) + Yt(idx,2) * Yt(idx,2) + Yt(idx,3) * Yt(idx,3);
        error_angle(idx,1) = radtodeg(acos(dotp/sqrt(lenSq1 * lenSq2))); % * RAD_TO_DEG;            
    end;
    
    for idx = 1:numel(prx)
        dotp = prx(idx) * Yt(idx,1) + pry(idx) * Yt(idx,2) + prz(idx) * Yt(idx,3);
        lenSq1 = prx(idx) * prx(idx) + pry(idx) * pry(idx) + prz(idx) * prz(idx);
        lenSq2 = Yt(idx,1) * Yt(idx,1) + Yt(idx,2) * Yt(idx,2) + Yt(idx,3) * Yt(idx,3);
        error_angle(idx,2) = radtodeg(acos(dotp/sqrt(lenSq1 * lenSq2))); % * RAD_TO_DEG;
    end;
    
    if( program_options.calc_vergence == 1)
        for idx = 1:numel(pvx)
            dotp = pvx(idx) * Yt(idx,1) + pvy(idx) * Yt(idx,2) + pvz(idx) * Yt(idx,3);
            lenSq1 = pvx(idx) * pvx(idx) + pvy(idx) * pvy(idx) + pvz(idx) * pvz(idx);
            lenSq2 = Yt(idx,1) * Yt(idx,1) + Yt(idx,2) * Yt(idx,2) + Yt(idx,3) * Yt(idx,3);
            error_angle(idx,3) = radtodeg(acos(dotp/sqrt(lenSq1 * lenSq2))); % * RAD_TO_DEG;
            error_dist(idx,1) = sqrt((pvx(idx)-Yt(idx,1))^2 + (pvy(idx)-Yt(idx,2))^2 + (pvz(idx)-Yt(idx,3))^2);
        end;
    end;
end;
%%%%%%


%%%%%% ANGLE CALCULATION
if( program_options.calc_angle == 1 )
    
    angle = zeros(size(prx,1),3);
    
    for idx = 1:numel(prx)
        x1 = eyesL(1); y1 = eyesL(2); z1 = eyesL(3);
        x2 = plx; y2 = ply; z2 = plz;
        x3 = eyesR(1); y3 = eyesR(2); z3 = eyesR(3);
        x4 = prx; y4 = pry; z4 = prz;
        %angle(idx,1) = atan2(abs((x2(idx)-x1)*(y4(idx)-y3)-(x4(idx)-x3)*(y2(idx)-y1)),(x2(idx)-x1)*(x4(idx)-x3)+(y2(idx)-y1)*(y4(idx)-y3));
        Va = [x2(idx)-x1 y2(idx)-y1 z2(idx)-z1];
        Vb = [x4(idx)-x3 y4(idx)-y3 z4(idx)-z3];
        Va = Va/norm(Va); Vb = Vb/norm(Vb);
        angle(idx,1) = radtodeg(atan2(norm(cross(Va,Vb)), dot(Va,Vb)));
%         Va = [ x2(idx)-x4(idx) ];
%         angle(idx,1) = mean(Va);
        if(program_options.calc_vergence)
            x1 = eyesL(1); y1 = eyesL(2); z1 = eyesL(3);
            x2 = pvx; y2 = pvy; z2 = pvz;
            x3 = eyesR(1); y3 = eyesR(2); z3 = eyesR(3);
            x4 = pvx; y4 = pvy; z4 = pvz;
            %angle(idx,1) = atan2(abs((x2(idx)-x1)*(y4(idx)-y3)-(x4(idx)-x3)*(y2(idx)-y1)),(x2(idx)-x1)*(x4(idx)-x3)+(y2(idx)-y1)*(y4(idx)-y3));
            Va = [x2(idx)-x1 y2(idx)-y1 z2(idx)-z1];
            Vb = [x4(idx)-x3 y4(idx)-y3 z4(idx)-z3];
            Va = Va/norm(Va); Vb = Vb/norm(Vb);
            angle(idx,2) = radtodeg(atan2(norm(cross(Va,Vb)), dot(Va,Vb)));
        end;
        x1 = eyesL(1); y1 = eyesL(2); z1 = eyesL(3);
        x2 = Yt(:,1); y2 = Yt(:,2); z2 = Yt(:,3);
        x3 = eyesR(1); y3 = eyesR(2); z3 = eyesR(3);
        x4 = Yt(:,1); y4 = Yt(:,2); z4 = Yt(:,3);
        %angle(idx,1) = atan2(abs((x2(idx)-x1)*(y4(idx)-y3)-(x4(idx)-x3)*(y2(idx)-y1)),(x2(idx)-x1)*(x4(idx)-x3)+(y2(idx)-y1)*(y4(idx)-y3));
        Va = [x2(idx)-x1 y2(idx)-y1 z2(idx)-z1];
        Vb = [x4(idx)-x3 y4(idx)-y3 z4(idx)-z3];
        Va = Va/norm(Va); Vb = Vb/norm(Vb);
        angle(idx,3) = radtodeg(atan2(norm(cross(Va,Vb)), dot(Va,Vb)));
    end;
end;
%%%%%%


%%%%%% PAINT
if( program_options.paint_plot == 1)
    %plot Points
    figure;
    set(gca, 'XDir', 'reverse')
    title(['Cal:' title_cal '    Est:' title_est]);
    grid on;
    grid minor;
    hold on;
    axes_limx = [-300 300 ];
    axes_limy = [0 600 ];
    axes_limz = [0 1500];
    axis([ axes_limx(1) axes_limx(2) axes_limy(1) axes_limy(2) axes_limz(1) axes_limz(2)]);
    %ax.XTick = [-500:100:500];
    % ax.YTick = 1;
    %ax.ZTick = [0:100:1200];
    % Add title and axis labels
    xlabel('X');
    ylabel('Y');
    zlabel('Z');
    
    
    %plot targets points
    h = scatter3(Yt(:,1), Yt(:,2), Yt(:,3),'filled');
    h.Marker = 'd';
    h.MarkerFaceColor = 'b';
    a = (1:length(Yt(:,1)))'; b = num2str(a); c = cellstr(b);
    dx = 1; dy = 1; % displacement so the text does not overlay the data points
    text(Yt(:,1)+dx, Yt(:,2)+dy, Yt(:,3), c, 'color', 'b');
    
    % plot the eyeballs
    h = scatter3(eyesL(1), eyesL(2), eyesL(3));
    h.MarkerFaceColor = [1 .5 0];
    h = scatter3(eyesR(1), eyesR(2), eyesR(3));
    h.MarkerFaceColor = 'r';
    
    % plot calibration plane
    patch_cal_axes_x = [ axes_limx(1) axes_limx(2) axes_limx(2) axes_limx(1)];
    patch_cal_axes_y = [ axes_limy(1) axes_limy(1) axes_limy(2) axes_limy(2)];
    patch_cal_axes_z = ones(1,numel(patch_cal_axes_x))* TargetPosCalZcal(1);
    patch_cal = patch(patch_cal_axes_x, patch_cal_axes_y, patch_cal_axes_z, 'g');
    set(patch_cal,'facealpha',0.1);
    set(patch_cal,'edgealpha',0.2);
    
    if( program_options.paint_targetcalib == 1 )
        %plot target calibration points
        h = scatter3(TargetPosCalXcal, TargetPosCalYcal, TargetPosCalZcal,'filled');
        h.Marker = 'd';
        h.MarkerFaceColor = 'b';
        a = (1:length(TargetPosCalXcal))'; b = num2str(a); c = cellstr(b);
        dx = 1; dy = 1; % displacement so the text does not overlay the data points
        text(TargetPosCalXcal+dx, TargetPosCalYcal+dy, TargetPosCalZcal, c, 'color', 'b');
    end;
    
    if( program_options.paint_point == 1 )
        %plot line between eyes and target point
        ptsL = [eyesL; [ Yt(point,1), Yt(point,2), Yt(point,3)] ];
        l = line(ptsL(:,1), ptsL(:,2), ptsL(:,3));
        l.Color = 'b';
        
        %plot lines to target from right eye
        ptsR = [eyesR; [ Yt(point,1), Yt(point,2), Yt(point,3)] ];
        l = line(ptsR(:,1), ptsR(:,2), ptsR(:,3));
        l.Color = 'b';
        
        %plot the interpolated points
        ptsL = [eyesL; [ pvx(point), pvy(point), pvz(point)] ];
        l = line(ptsL(:,1), ptsL(:,2), ptsL(:,3));
        l.Color = [1 .5 0];
        ptsR = [eyesR; [ pvx(point), pvy(point), pvz(point)] ];
        l = line(ptsR(:,1), ptsR(:,2), ptsR(:,3));
        l.Color = 'r';
        %         %plot the interpolated points
        %         ptsL = [eyesL; [ plx(point), ply(point), plz(point)] ];
        %         l = line(ptsL(:,1), ptsL(:,2), ptsL(:,3));
        %         l.Color = [1 .5 0];
        %         ptsR = [eyesR; [ prx(point), pry(point), prz(point)] ];
        %         l = line(ptsR(:,1), ptsR(:,2), ptsR(:,3));
        %         l.Color = 'r';
        
        
        
        %plot Intersection point between calibration plane and line from eyes to target point
        [Ir,check]=plane_line_intersect([0 0 1] , [0, 0, TargetPosCalZcal(1)], eyesR, [Yt(point,1) Yt(point,2) Yt(point,3)]);
        if( check == 1)
            h = scatter3(Ir(1), Ir(2), Ir(3));
            h.Marker = '*';
            h.MarkerFaceColor = 'r';
        end;
        [Il,check]=plane_line_intersect([0 0 1] , [0, 0, TargetPosCalZcal(1)], eyesL, [Yt(point,1) Yt(point,2) Yt(point,3)]);
        if( check == 1 )
            h = scatter3(Il(1), Il(2), Il(3));
            h.Marker = '*';
            h.MarkerFaceColor = [1 .5 0];
        end;
    end;
    
    if( program_options.paint_intp == 1)
        h = scatter3(plx, ply, plz);
        h.Marker = 'o';
        h.MarkerFaceColor = [1 .5 0];
        a = (1:length(plx))'; b = num2str(a); c = cellstr(b);
        dx = 1; dy = 1; % displacement so the text does not overlay the data points
        text(plx+dx, ply+dy, plz, c, 'color', [1 .5 0]);
        
        h = scatter3(prx, pry, prz);
        h.Marker = 'o';
        h.MarkerFaceColor = 'r';
        a = (1:length(prx))'; b = num2str(a); c = cellstr(b);
        dx = 1; dy = 1; % displacement so the text does not overlay the data points
        text(prx+dx, pry+dy, prz, c, 'color', 'r');
    end
    
    if( program_options.paint_vergence == 1 && program_options.calc_vergence == 1 )
        h = scatter3(pvx, pvy, pvz);
        h.Marker = 'o';
        h.MarkerFaceColor = 'k';
        a = (1:length(pvx))'; b = num2str(a); c = cellstr(b);
        dx = 1; dy = 1; % displacement so the text does not overlay the data points
        text(pvx+dx, pvy+dy, pvz, c, 'color', 'k');
    end;
end;

if( program_options.paint_error == 1 && program_options.calc_error == 1 )
    figure;
    hold on;
    title(['Error of Angle   Cal:' title_cal '    Est:' title_est]);
    color = {[1 .5 0],'r','k'};
    leg = {'intp. L', 'intp. R', 'Vergence'};
    for i=1:3
        plot(error_angle(:,i), 'color',color{i});
    end;
    ax = gca;
    ax.XTick = [1:size(error_angle,1)];
    legend(leg);
    hold off;
    
    figure;
    hold on;
    title(['Error of Distance   Cal:' title_cal '    Est:' title_est]);
    color = {[1 .5 0],'r',};
    plot(error_dist(:,1), 'color','k');
    ax = gca;
    ax.XTick = [1:size(error_angle,1)];
    hold off;
end;
%%%%%%