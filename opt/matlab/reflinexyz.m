function hxy = reflinexyz(x,y,z,varargin)
% REFLINEXYZ - Plot 3D reference lines to the YZ-, XZ- and XY-planes
%   REFLINEXYZ(X,Y,Z) plots reference lines from the 3D points (X,Y,Z) to
%   the three cartesian planes (YZ, XZ, & XY planes). The lines are drawn
%   from the axes planes upto each point. X, Y and Z should have the same
%   number of elements. 
%
%   REFLINEXYZ(X,Y,Z,YZOFFSET, XZOFFSET, XYOFFSET) specifies the offset
%   from the three planes. If an offset is omitted (or empty), the
%   defaults are used, namely the lower limits of each axis. The offsets
%   can be scalars, specifying the same offset for each point, or vectors
%   with the same number of elements as the points, in order to specify the
%   offset for each point individually. 
%
%   The lines are plotted as a three graphics object. H = REFLINEXYZ(..) returns
%   the three graphics handles of the lines to the YZ, XZ and XY planes in H. 
%
%   REFLINEXYZ(..., 'Prop1','Val1','Prop2','Val2', ...) uses the properties
%   and values specified for color, linestyle, etc. Execute GET(H), where H is
%   a line handle, to see a list of line object properties and their current values.
%   Execute SET(H) to see a list of line object properties and legal property values.
%   REFLINEXYZ uses the current axes, if any. Lines outside the plot area
%   are plotted but not shown.
%
%   Example:
%     % create and plot some data
%       N = 50 ; i=0:N ;
%       x = (i/N) * 2 * pi ;
%       y = exp(i/N) .* sin(3*x) ;
%       z = (2*i/N).^2 ;
%       plot3(x,y,z,'bo-') ;
%       xlabel('X-axis') ; ylabel('Y-axis') ; zlabel('Z-axis') ; grid on ;
%     % add specific reference lines
%       i0 = [10 20] ; % indices of points
%       h = reflinexyz(x(i0),y(i0),z(i0),'color','r','linestyle','-','linewidth',3,'marker','.') ;
%       title ('Reference lines to 2 points') ;
%       disp('Press a key') ; pause ; 
%     % make a projection to xy-plane
%       hold on ; plot3(x,y,zeros(size(x)),'k.-') ; hold off ;
%       h = reflinexyz(x,y,z,'color','k','linestyle','-') ; 
%       delete(h([1 2])) ; % delete the yz and xz projections
%       title ('Projection to XY plane') ;
%
%   See also STEM, PLOT3, REFLINE, GRID, AXES
%   and GRIDXY and REFLINEXY (on the FEX)

% for Matlab R13
% version 1.2 (okt 2007)
% (c) Jos van der Geest
% email: jos@jasen.nl

% History:
% 1.0 (okt 2007) - created
% 1.1 (okt 2007) - small code revisions (variable names) and added comments
% 1.2 (okt 2007) - removed spelling and grammar errors

error(nargchk(3,Inf,nargin)) ;

% get the axes to plot in
hca=get(get(0,'currentfigure'),'currentaxes');
if isempty(hca),
    warning('No current axes found') ;
    return ;
end

% check the arguments
if ~isnumeric(x) || ~isnumeric(y) || ~isnumeric(z),
    error('Three numeric arguments expected') ;
end

N = numel(x) ;

if ~isequal(N,numel(y), numel(z)),
    error('X, Y and Z should have the same number of elements') ;
end

% assume that the reference are to be draw from the "bottom" of the planes
xyz0(1:3) = {[]} ; 



if nargin==3,
    va = [] ;
else
    % Parse the first 3 arguments arguments
    va = varargin ;
    for i=1:3,
        tmp = va{1} ;        
        if isnumeric(tmp),
            % numeric argument specifies offset
            if ~isempty(tmp),                
                if numel(tmp)==1,
                    xyz0{i} = repmat(tmp,N,1) ;
                elseif numel(tmp) ~= N,
                    error('Offsets should have the same number of elements as the 3D points (or be scalars).') ;
                else
                    xyz0{i} = tmp(:) ;
                end               
                va = va(2:end) ;
            end
        elseif ischar(tmp),
            % a char-array wil specify line properties, so break
            break ; 
        else
            error('Parameter %d is invalid',i+3) ;
        end
    end       
end

% minimum check for property-value pairs
if mod(size(va),2) == 1,
    error('Property-Value have to be pairs') ;
end

% get the current limits of the axis
% also used for limit restoration later on
xyzlim{1} = get(hca,'xlim') ;
xyzlim{2} = get(hca,'ylim') ;
xyzlim{3} = get(hca,'zlim') ;

% create offsets for the three planes
for i=1:3,
    if isempty(xyz0{i}),
        tmp = xyzlim{i} ;
        xyz0{i} = repmat(tmp(1),1,N) ;
    end
    tmp = xyz0{i} ;
    xyz0{i} = tmp(:).' ; % column vector
end

if N,   
    nanfill = repmat(nan,1,N) ;
    
    % build the lines
    xx = repmat(x(:).',3,1) ; xx = xx(:) ;
    yy = repmat(y(:).',3,1) ; yy = yy(:) ;
    zz = repmat(z(:).',3,1) ; zz = zz(:) ;
    xxnan = [xyz0{1} ; x(:).' ; nanfill] ;
    yynan = [xyz0{2} ; y(:).' ; nanfill] ;
    zznan = [xyz0{3} ; z(:).' ; nanfill] ;
        
    % add the line to the current axes
    np = get(hca,'nextplot') ;
    set(hca,'nextplot','add') ;
    hxy(1) = line('xdata',xxnan(:),'ydata',yy,'zdata',zz,'linestyle',':','color','k') ; % draw lines to yz-plane
    hxy(2) = line('xdata',xx,'ydata',yynan(:),'zdata',zz,'linestyle',':','color','k') ; % draw lines to xz plane
    hxy(3) = line('xdata',xx,'ydata',yy,'zdata',zznan(:),'linestyle',':','color','k') ; % draw lines to xy plane
        
    uistack(hxy,'bottom') ; % push lines to the bottom of the graph
    
    set(hca,'nextplot',np) ;    % reset the nextplot state
    set(hca,'xlim',xyzlim{1},'ylim',xyzlim{2},'zlim',xyzlim{3}) ; % reset the limits
    
    if ~isempty(va),
        try
            set(hxy,va{:}) ; % set line properties        
        catch
            % invalid arguments, modify error message
            delete(hxy) ;
            msg = lasterror ;
            error(msg.message(21:end)) ;    
        end
    end
else
    hxy = [] ;
end

% if requested return handles
if ~nargout,     
    clear hxy ;
end
