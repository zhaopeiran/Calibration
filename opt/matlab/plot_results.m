save_points = 0;
paint_sta = 0;
paint_interp = 1;
paint_vergen = 1;

person = 'Peiran12';
interpupilar = 63;
eyesY = 410;
eyesZ = 1200;

folder_in  = ['/home/esguti/Dev/Eyetracking/Calibration/calibration_in/test12/' person];
folder_out = ['/home/esguti/Dev/Eyetracking/Calibration/calibration_out/test12/' person];

file_tar     = [folder_in '/test_' person '_target_pos.txt'];
file_both    = [folder_out '/botheyes.csv'];
file_pos     = [folder_out '/out_Gazetracker.csv'];
file_sta     = [folder_out '/out_Stadistics.csv'];
file_img_out = [folder_out '/calib_image_' person '_'];

calib_tar_start = 1;
calib_tar = 25;
point = 25;

eyesR = [interpupilar/2 eyesY eyesZ];
eyesL = [-interpupilar/2 eyesY eyesZ];
Mboth = dlmread(file_both,';',0,0);
BothRightPosX = Mboth(:,1);
BothRightPosY = Mboth(:,2);
BothLeftPosX  = Mboth(:,3);
BothLeftPosY  = Mboth(:,4);
Msta = dlmread(file_sta,',',1,0);
stadeg = Msta(:,1);
Mtar = dlmread(file_tar,';',1,0);
TargetPosX = Mtar(:,2);
TargetPosY = Mtar(:,3);
TargetPosZ = Mtar(:,4);
Mpos = dlmread(file_pos,';',1,0);
posX = Mpos(:,2);
posY = Mpos(:,3);
posZ = Mpos(:,4);

%plot stadistics
if ( paint_sta )
    gcf = figure;
    title('Error (deg)');
    grid on;
    grid minor;
    plot(stadeg(calib_tar_start:calib_tar));
    % set(gca, 'YTick', [min(stadeg):1:max(stadeg)]);
    set(gca, 'XTick', [calib_tar_start:1:calib_tar]);
    saveas( gcf, [file_img_out 'sta.jpg'] );
    close;
end;

%plot Points
figure;
title('Points');
grid on;
grid minor;
hold on;
axis([-300 300 50 550 0 1500]);
% ax.XTick = [-500:1:500];
% ax.YTick = 1;
% ax.ZTick = 1;
% Add title and axis labels
xlabel('X');
ylabel('Y');
zlabel('Z');

% plot the eyeballs
scatter3(eyesL(1), eyesL(2), eyesL(3));
scatter3(eyesR(1), eyesR(2), eyesR(3));

%plot lines to target from left eye
% ptsL = [eyesL; [ TargetPosX(point), TargetPosY(point), TargetPosZ(point)] ];
% l = line(ptsL(:,1), ptsL(:,2), ptsL(:,3));
% l.Color = 'b';

%plot lines to target from right eye
% ptsR = [eyesR; [ TargetPosX(point), TargetPosY(point), TargetPosZ(point)] ];
% l = line(ptsR(:,1), ptsR(:,2), ptsR(:,3));
% l.Color = 'b';

%plot target points
h = scatter3(TargetPosX(calib_tar_start:calib_tar), TargetPosY(calib_tar_start:calib_tar), TargetPosZ(calib_tar_start:calib_tar),'filled');
h.Marker = 'd';
h.MarkerFaceColor = 'b';
a = (calib_tar_start:calib_tar)'; b = num2str(a); c = cellstr(b);
dx = 1; dy = 1; % displacement so the text does not overlay the data points
text(TargetPosX(calib_tar_start:calib_tar)+dx, TargetPosY(calib_tar_start:calib_tar)+dy, TargetPosZ(calib_tar_start:calib_tar), c, 'color', 'b');

if( paint_interp )
    %plot interpolation left point
    h = scatter3(BothLeftPosX(calib_tar_start:calib_tar), BothLeftPosY(calib_tar_start:calib_tar), TargetPosZ(calib_tar_start:calib_tar),'filled');
    h.Marker = 's';
    h.MarkerFaceColor = [1 .5 0];
    a = (calib_tar_start:calib_tar)'; b = num2str(a); c = cellstr(b);
    dx = 1; dy = 1; % displacement so the text does not overlay the data points
    text(BothLeftPosX(calib_tar_start:calib_tar)+dx, BothLeftPosY(calib_tar_start:calib_tar)+dy, TargetPosZ(calib_tar_start:calib_tar), c, 'color', 'b');
    
    %plot interpolation right point
    h = scatter3(BothRightPosX(calib_tar_start:calib_tar), BothRightPosY(calib_tar_start:calib_tar), TargetPosZ(calib_tar_start:calib_tar),'filled');
    h.Marker = 's';
    h.MarkerFaceColor = 'r';
    a = (calib_tar_start:calib_tar)'; b = num2str(a); c = cellstr(b);
    dx = 1; dy = 1; % displacement so the text does not overlay the data points
    text(BothRightPosX(calib_tar_start:calib_tar)+dx, BothRightPosY(calib_tar_start:calib_tar)+dy, TargetPosZ(calib_tar_start:calib_tar), c, 'color', 'b');
    
    %plot lines to Left point
    ptsL = [eyesL; eyesL; [ BothLeftPosX(point), BothLeftPosY(point), TargetPosZ(point)] ];
    px = ptsL(1,1); py = ptsL(1,2); pz = ptsL(1,3);
    l = plot3(ptsL(:,1), ptsL(:,2), ptsL(:,3));
    l.Color = [1 .5 0];
    [x0, slope ] = ls3dline(ptsL);
    PL = zeros(1000,3); for t=1:1000; PL(t,:) = [px;py;pz] + slope.*t; end;
    l = plot3(PL(:,1),PL(:,2),PL(:,3));
    l.Color = [1 .5 0];
    

    %plot lines to Right point
    ptsR = [eyesR; eyesR; [ BothRightPosX(point), BothRightPosY(point), TargetPosZ(point)] ];
    px = ptsR(1,1); py = ptsR(1,2); pz = ptsR(1,3);
    l = plot3(ptsR(:,1), ptsR(:,2), ptsR(:,3));
    l.Color = 'r';
    [x0, slope ] = ls3dline(ptsR);
    PR = zeros(1000,3); for t=1:1000; PR(t,:) = [px;py;pz] + slope.*t; end;
    l = plot3(PR(:,1),PR(:,2),PR(:,3));
    l.Color = 'r';
end

if( paint_vergen )
    % plot vergence point
    % posZ(calib_tar_start:calib_tar) = TargetPosZ(calib_tar_start:calib_tar);
    h = scatter3(posX(calib_tar_start:calib_tar), posY(calib_tar_start:calib_tar), posZ(calib_tar_start:calib_tar),'filled');
    h.Marker = 'o';
    h.MarkerFaceColor = 'k';
    a = [calib_tar_start:calib_tar]'; b = num2str(a); c = cellstr(b);
    dx = 1; dy = 1; % displacement so the text does not overlay the data points
    text(posX(calib_tar_start:calib_tar)+dx, posY(calib_tar_start:calib_tar)+dy, posZ(calib_tar_start:calib_tar), c, 'color', 'k');
end

if ( save_points )
    title('Superior View')
    view(0, 0);
    saveas(h,[file_img_out 'sup.jpg']);
    title('Lateral View')
    view(90, 0);
    camroll(90);
    saveas(h,[file_img_out 'lat.jpg']);
    title('Front View')
    view(0, 90);
    saveas(h,[file_img_out 'front.jpg']);
end;
% hold off;
% close all;