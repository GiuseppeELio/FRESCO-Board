%% Matlab code to analyse FRESCO data

%*************************************************************
%                       Initialisation
%*************************************************************

clear all           % Clear all variables from the workspace
close all           % Close all open figure windows
clc                 % Clear the command window
set(0, 'DefaultLineLineWidth', 2);  % Set default line width for all plots

% Display script information in the command window
disp(['-------------Matlab code developed by------------', newline, ...
      '-----------------Jérémy Werlé LENS ---------------', newline, ...
      '---------------FRESCO DATA ANALYSIS---------------'])

%%
%*************************************************************
%                       Unzip file
%*************************************************************

% /!\ Unzip only necessary for this example data, 
% not needed for standard FRESCO data /!\ 

unzip('../Experimental_Data/data_sample.zip', '../Experimental_Data');

%%
%*************************************************************
%                       Create data file
%*************************************************************

%-------------------------------------------------------------
% Create a UNIQUE DATA.txt file containing all the data between two dates
%-------------------------------------------------------------

% Define the start date for data extraction
start_day = 14;
start_month = 7;
start_year = 2024;

% Define the end date for data extraction
end_day = 17;
end_month = 7;
end_year = 2024;

% Define the output filepath
merged_filepath = fullfile('..','Experimental_Data','Data_Merged.txt');

% Define folder path containing unzipped data
data_path = '../Experimental_Data';

% Check if the output file already exists
if exist(merged_filepath, 'file') == 2
    disp('Merged file exists in the working directory.');
else
    disp('Merged file does not exist in the working directory.');
    
    % Call the custom function to read and combine the data
    % Function is assumed to be located in the Scripts folder
    read_data_from_folders(data_path, start_day, start_month, start_year, ...
                           end_day, end_month, end_year,merged_filepath);
end

%%
%*************************************************************
%                 Timetable creation
%*************************************************************

%-------------------------------------------------------------
% Read the txt file into a table with appropriate import options
%-------------------------------------------------------------

opts = detectImportOptions(merged_filepath, 'Delimiter', ',', 'PreserveVariableNames', true);  % Auto-detect options
opts = setvartype(opts, 1, 'datetime');                             % Set the first column (timestamp) as datetime
opts = setvaropts(opts, 1, 'InputFormat', 'dd/MM/yyyy HH:mm:ss');   % Specify expected datetime format
opts.ExtraColumnsRule = 'ignore';                                   % Ignore any unexpected extra columns
opts.ConsecutiveDelimitersRule = 'join';                            % Merge multiple delimiters into one

df = readtable(merged_filepath, opts);     % Read the data into a table using the options

%-------------------------------------------------------------
% Convert the imported table into a timetable for time-based analysis
%-------------------------------------------------------------
df = table2timetable(df);          % Convert to timetable for easier time-series processing

%% 
%*************************************************************
%              Compute temperature drops
%*************************************************************

%-------------------------------------------------------------
% Define the reference ambient temperature
%-------------------------------------------------------------
df.Ref = df.TA1;  % Use TA1 as the reference ambient temperature
% Alternative method: use the coldest between TA1, TA2, and TA3
% df.Ref = min([df.TA1, df.TA2, df.TA3], [], 2);

%-------------------------------------------------------------
% Compute temperature differences (ΔT) relative to reference
%-------------------------------------------------------------
df.DeltaT1    = df.TS1   - df.Ref;   % Sample 1 temperature drop
df.DeltaT2    = df.TS2   - df.Ref;   % Sample 2 temperature drop
df.DeltaT3    = df.TS3   - df.Ref;   % Under Surface 1 temperature drop
df.DeltaT4    = df.TS4   - df.Ref;   % Under Surface 2 temperature drop
df.DeltaTbx   = df.Tbx   - df.Ref;   % Lid temperature drop
df.DeltaTPC1  = df.TPC1  - df.Ref;   % Sample 1 PCool temperature drop
df.DeltaTPC2  = df.TPC2  - df.Ref;   % Sample 2 PCool temperature drop

%%
%*************************************************************
%            Timetable sliding average and resampling
%*************************************************************

%-------------------------------------------------------------
% Define sliding window size
%-------------------------------------------------------------
ws = 15;                                     % Window size in minutes
window_size = minutes(ws);                   % Convert to duration object

% Estimate the sampling interval (median time step between rows)
sampling_interval = median(diff(df.("Date & Time")), 'omitmissing');  

% Calculate window size in number of samples
window_size_samples = round(window_size / sampling_interval);

%-------------------------------------------------------------
% Apply sliding average (moving mean) to all numeric variables
%-------------------------------------------------------------
Timetable = varfun(@(x) movmean(x, window_size_samples), df, ...
                   'InputVariables', @isnumeric);     % Apply movmean only to numeric variables
Timetable.Properties.VariableNames = df.Properties.VariableNames;  % Preserve original variable names

%-------------------------------------------------------------
% Optional: downsample every 5-minutes for plotting efficiency
%-------------------------------------------------------------
Timetable = retime(df, 'regular', 'mean', 'TimeStep', minutes(5));  % Resample data with 5-minute mean

%% 
%*************************************************************
%                     Create TDrop plots
%*************************************************************

% Define custom time range for x-axis limits
graph_time_limit = [Timetable.("Date & Time")(1), Timetable.("Date & Time")(end)];

% Create figure with 4 subplots for TDrop analysis
figure;
sgtitle('TDrop Plots', Interpreter='latex');  % Super title for the figure

%-------------------------------------------------------------
% 1st Plot: Absolute temperatures
%-------------------------------------------------------------
ax1 = subplot(2, 2, 1);
hold on;
plot(ax1, Timetable.("Date & Time"), Timetable{:, {'TS1','TS2', 'TS3','TS4','Tbx'}});
plot(Timetable.("Date & Time"), Timetable.Ref,'k--');

legend({'S1', 'S2','Under S1', 'Under S2','T Lid','$T_{amb}$'}, ...
       'Location', 'southeast', 'Interpreter', 'latex');
ylabel('Temperature [$^\circ$C]', Interpreter='latex');
ax1.XLim = graph_time_limit;
grid on;

%-------------------------------------------------------------
% 2nd Plot: Solar Irradiance and T_sky
%-------------------------------------------------------------
ax2 = subplot(2, 2, 2);
hold on

yyaxis left
area(Timetable.("Date & Time"), Timetable.Ir, ...
     'FaceAlpha', 0.2, 'EdgeColor', 'none', 'FaceColor', '#1f77b4');
ylabel('Solar Irradiance [W/m$^2$]', 'Color', '#1f77b4', Interpreter='latex');

yyaxis right
plot(Timetable.("Date & Time"), Timetable.TSIR, '--', 'Color', '#d62728');
ylabel('T$_{sky}$ [$^\circ$C]', Interpreter="latex");
ax2.XLim = graph_time_limit;
grid on;

%-------------------------------------------------------------
% 3rd Plot: Delta T for S1 and S2 vs Irradiance
%-------------------------------------------------------------
ax3 = subplot(2, 2, 3);
plot(Timetable.("Date & Time"), Timetable{:, {'DeltaT1', 'DeltaT2'}});
yline(0, 'k--', 'LineWidth', 2, 'HandleVisibility', 'off');  % Horizontal zero line
ylim([-8 8])
ylabel('$\Delta$T = T$_{sample}$ - T$_{amb}$ [$^\circ$C]', ...
       'Color', 'k', 'Interpreter', 'latex');
legend({'$\Delta$T S1', '$\Delta$T S2'}, ...
       'Location', 'southeast', 'Interpreter', 'latex');
grid on;

% Overlay irradiance on right y-axis
yyaxis right
ax3 = gca;
ax3.YColor = '#1f77b4';
area(Timetable.("Date & Time"), Timetable.Ir, ...
     'FaceAlpha', 0.2, 'EdgeColor', 'none', 'FaceColor', '#1f77b4', ...
     'HandleVisibility', 'off');
ylabel('Solar Irradiance [W/m$^2$]', 'Color', '#1f77b4', 'Interpreter', 'latex');
ax3.XLim = graph_time_limit;

%-------------------------------------------------------------
% 4th Plot: Remlative Ambient Humidity vs Irradiance
%-------------------------------------------------------------
ax4 = subplot(2, 2, 4);
plot(Timetable.("Date & Time"), Timetable.H1,'k-');
ylabel('Relative Ambient Humidity [$\%$]','Color', 'k', 'Interpreter', 'latex');

% Overlay irradiance on right y-axis
yyaxis right
ax4 = gca;
ax4.YColor = '#1f77b4';
area(Timetable.("Date & Time"), Timetable.Ir, ...
     'FaceAlpha', 0.2, 'EdgeColor', 'none', 'FaceColor', '#1f77b4', ...
     'HandleVisibility', 'off');
ylabel('Solar Irradiance [W/m$^2$]', 'Color', '#1f77b4', 'Interpreter', 'latex');
grid on;


ax4.XLim = graph_time_limit;

%-------------------------------------------------------------
% Add x-axis labels
%-------------------------------------------------------------
xlabel(ax3, 'Date \& Time', Interpreter='latex');
xlabel(ax4, 'Date \& Time', Interpreter='latex');

%% 
%*************************************************************
%                     Create PCool plots
%*************************************************************

% Create a new figure with 2 subplots to display passive cooling data
figure;
sgtitle('PCool Plots', Interpreter='latex');  % Title for the entire figure

%-------------------------------------------------------------
% 1st Subplot: Plot ambient and TPC temperatures
%-------------------------------------------------------------
ax5 = subplot(2, 1, 1);
grid on;
hold on;

% Plot ambient temperature
plot(Timetable.("Date & Time"), Timetable{:, 'TA1'}, 'Color', 'k', 'LineWidth', 1.5);

% Plot temperatures from two TPC (passive cooling) samples
plot(Timetable.("Date & Time"), Timetable{:, 'TPC1'}, 'Color', 'b', 'LineWidth', 1.5);
plot(Timetable.("Date & Time"), Timetable{:, 'TPC2'}, 'Color', 'r', 'LineWidth', 1.5);

ylabel('Temperature [$^\circ$C]', Interpreter='latex');

% Add legend with LaTeX labels
legend({'T$_{amb}$', 'TPC S1', 'TPC S2'}, ...
       'Location', 'southwest', 'Interpreter', 'latex');

% Set limits for x and y axes
ax5.XLim = graph_time_limit;
ax5.YLim = [15 42];  % Temperature range in °C

%-------------------------------------------------------------
% 2nd Subplot: Plot power density and solar irradiance
%-------------------------------------------------------------
ax6 = subplot(2, 1, 2);
hold on;

% Plot power density from two TPC surfaces
plot(Timetable.("Date & Time"), Timetable{:, 'PD1'}, 'b--', 'LineWidth', 1.5);
plot(Timetable.("Date & Time"), Timetable{:, 'PD2'}, 'r--', 'LineWidth', 1.5);

% Left y-axis for power density
ylabel('Power Density [W/m$^2$]', 'Interpreter', 'latex', 'Color', 'k');

% Add solar irradiance on the right y-axis
yyaxis right;
ax7 = gca;
ax7.YColor = '#1f77b4';  % Set color of right y-axis

% Fill area under irradiance curve for visual clarity
area(Timetable.("Date & Time"), Timetable.Ir, ...
     'FaceAlpha', 0.2, 'EdgeColor', 'none', ...
     'FaceColor', '#1f77b4', 'HandleVisibility', 'off');

% Right y-axis label
ylabel('Solar Irradiance [W/m$^2$]', ...
       'Color', '#1f77b4', 'Interpreter', 'latex');

% Add legend for power density plots
legend({'PD S1', 'PD S2'}, ...
       'Location', 'southwest', 'Interpreter', 'latex');

% Set limits for x-axis
ax6.XLim = graph_time_limit;

% Label x-axis with LaTeX
xlabel(ax6, 'Date \& Time', Interpreter='latex');

%% 
%*************************************************************
%                     Create Violin Plot
%*************************************************************

% Clear specific variables that might hold old data
clear night_mask day_mask daytime_data nighttime_data data_day_all ...
    My_mean_day data_night_all

%-------------------------------------------------------------
% Define the time ranges to segment the data into day and night
%-------------------------------------------------------------
% Daytime: from 11:00 to 16:00
day_start_hour = 11;
day_end_hour = 16;

% Nighttime: from 22:00 to 06:00 (next day)
night_start_hour = 22;
night_end_hour = 6;

%-------------------------------------------------------------
% Specify which variables (columns) from the timetable to analyze
%-------------------------------------------------------------
column_names = {'DeltaT1', 'DeltaT2', 'DeltaT3', 'DeltaT4'};  % ΔT values
titles = {'S1', 'S2', 'Under S1', 'Under S2'};                % Labels for plots

%-------------------------------------------------------------
% Create logical masks to separate data into day and night
%-------------------------------------------------------------
day_mask = (hour(Timetable.("Date & Time")) >= day_start_hour & ...
            hour(Timetable.("Date & Time")) < day_end_hour);

% Night mask considers values from 22:00 to 06:00 (overnight)
night_mask = (hour(Timetable.("Date & Time")) >= night_start_hour & ...
              hour(Timetable.("Date & Time")) >= night_end_hour) | ...
             (hour(Timetable.("Date & Time")) <= night_start_hour & ...
              hour(Timetable.("Date & Time")) < night_end_hour);

%-------------------------------------------------------------
% Initialize cell arrays to store extracted data
%-------------------------------------------------------------
daytime_data = cell(length(column_names), 1);
nighttime_data = cell(length(column_names), 1);

%-------------------------------------------------------------
% Loop over each variable name to extract and compute statistics
%-------------------------------------------------------------
for i = 1:length(column_names)
    col = column_names{i};  % Get variable name (e.g., 'DeltaT1')

    % Extract daytime data for the current column
    daytime_data{i} = Timetable{day_mask, col};
    data_day_all(:, i) = daytime_data{i};                          % Store for plotting
    My_mean_day(i) = mean(daytime_data{i}, 'omitnan');             % Compute mean

    % Extract nighttime data for the current column
    nighttime_data{i} = Timetable{night_mask, col};
    data_night_all(:, i) = nighttime_data{i};                      % Store for plotting
    My_mean_night(i) = mean(nighttime_data{i}, 'omitnan');         % Compute mean
end

%% 
%*************************************************************
%            Figure using the Boxchart style
%*************************************************************

figure; % Create a new figure window

%-------------------------------------------------------------
% Plot for daytime data (first subplot)
%-------------------------------------------------------------
subplot(2, 1, 1); % Create the first subplot (top half of the figure)
hold on;           % Keep current plot active to add multiple elements
grid on;           % Add grid to the plot

% Loop through each variable (column) for daytime data
for i = 1:length(column_names)
    data_day = daytime_data{i};  % Get the daytime data for current column

    % Create a categorical variable for grouping the data (each group is one of the titles)
    group_day = categorical(repmat(titles(i), size(data_day)));
    
    % Create a boxchart for the data (similar to a boxplot)
    boxchart(group_day, data_day);
    
    % Calculate the mean of the current data and plot it as a red star
    mean_day = mean(data_day);
    plot(i, mean_day, 'r*');
end

% Add a horizontal line at y=0 for reference (black dashed line)
yline(0, 'k--', 'LineWidth', 2);

% Add title and labels
title('Daytime Data', 'Interpreter', 'latex');
ylabel('$\Delta$T [$^\circ$C]', 'Interpreter', 'latex');

hold off;  % End plotting for this subplot

%-------------------------------------------------------------
% Plot for nighttime data (second subplot)
%-------------------------------------------------------------
subplot(2, 1, 2); % Create the second subplot (bottom half of the figure)
hold on;           % Keep current plot active to add multiple elements
grid on;           % Add grid to the plot

% Loop through each variable (column) for nighttime data
for i = 1:length(column_names)
    data_night = nighttime_data{i};  % Get the nighttime data for current column

    % Create a categorical variable for grouping the data (each group is one of the titles)
    group_night = categorical(repmat(titles(i), size(data_night)));
    
    % Create a boxchart for the data
    boxchart(group_night, data_night);
    
    % Calculate the mean of the current data and plot it as a red star
    mean_night = mean(data_night);
    plot(i, mean_night, 'r*');
end

% Add a horizontal line at y=0 for reference (black dashed line)
yline(0, 'k--', 'LineWidth', 2);
ylabel('$\Delta$T [$^\circ$C]', 'Interpreter', 'latex');

% Add title and labels


%% 
%*************************************************************
%              Violin plots using daviolinplot style
%*************************************************************

% Using function from https://github.com/povilaskarvelis/DataViz
    
condition_names = {'Day time', 'Night time'};  % Define the condition names (Daytime and Nighttime)

%-------------------------------------------------------------
% Ensure all data lists have the same length (pad with NaN if needed)
%-------------------------------------------------------------
max_length = max(cellfun(@length, [daytime_data; nighttime_data]));  % Get the maximum length between daytime and nighttime data

for i = 1:length(column_names)  % Loop through each column (temperature or other variables)
    % If daytime data is shorter than max_length, pad with NaN
    if length(daytime_data{i}) < max_length
        daytime_data{i}(end+1:max_length) = NaN;
        My_mean_day(i) = mean(daytime_data{i}, 'omitnan');  % Update the mean for daytime data
    end
    % If nighttime data is shorter than max_length, pad with NaN
    if length(nighttime_data{i}) < max_length
        nighttime_data{i}(end+1:max_length) = NaN;
        My_mean_night(i) = mean(nighttime_data{i}, 'omitnan');  % Update the mean for nighttime data
    end
end

%-------------------------------------------------------------
% Combine the daytime and nighttime data for plotting
%-------------------------------------------------------------
data_combined = [];  % Initialize an empty array for combined data

% Loop through each column and concatenate the corresponding daytime and nighttime data
for i = 1:length(column_names)
    data_combined = [data_combined; daytime_data{i}, nighttime_data{i}];  % Combine data into one array
end

%-------------------------------------------------------------
% Create the group labels for each condition (daytime or nighttime)
%-------------------------------------------------------------
n_columns = length(column_names);  % Number of columns (data variables)
n_rows = size(daytime_data{1}, 1);  % Number of rows (samples in each condition)

group_combined = [];  % Initialize the group variable

% For each column, assign group labels (1 for Day, 2 for Night)
for k = 1:n_columns
    group_combined = [group_combined, k * ones(1, n_rows)];  % Group labels for each condition
end

%-------------------------------------------------------------
% Set up custom color order for the plot (optional)
%-------------------------------------------------------------
currentColorOrder = get(gca, 'colororder');  % Get the current color order used by MATLAB

% Uncomment the following lines if you'd like to add custom colors:
% additionalColor = [0.5, 0.5, 0.5];  % Example grey color
% additionalColor2 = [0.75, 0.75, 0];  % Example yellow color
% additionalColor3 = [0.75, 0, 0.75];  % Example purple color

% Combine the current color order with additional colors
newColorOrder = [currentColorOrder];  % Here, you can extend it if needed

%-------------------------------------------------------------
% Plot the Violin Plot using the daviolinplot function
%-------------------------------------------------------------
figure;  % Create a new figure window

% Call the daviolinplot function to create the violin plots
daviolinplot(data_combined(:,:), 'groups', group_combined, ...
    'xtlabels', condition_names, 'violin', 'full', ...  % Full violins with the condition labels
    'boxcolors', 'w', 'color', newColorOrder, 'legend', titles);  % Box colors white and custom color order

% Add a horizontal line at y=0 for reference (black dashed line)
yline(0, 'k--', 'LineWidth', 2, 'HandleVisibility', 'off');

% Add title and labels
title(['Night time from ', num2str(night_start_hour), 'h to ', ...
    num2str(night_end_hour), 'h & Day time from ', num2str(day_start_hour), 'h to ', ...
    num2str(day_end_hour), 'h']);
ylabel('ΔT [°C]');

% Set the font size and adjust axis limits for better visualization
set(gca, 'FontSize', 12);
xl = xlim;  % Get the current x-axis limits
xlim([xl(1), xl(2) + 0.25]);  % Extend the x-axis limits slightly for better spacing
grid on;  % Enable grid lines for better visibility
