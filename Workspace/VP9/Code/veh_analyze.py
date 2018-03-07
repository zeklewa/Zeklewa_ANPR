# Include libs
from scipy import stats

# Read file
f = open('/home/zeklewa/Workspace/VP9/Asset/test_calibration.txt', 'r')
data = f.readlines()

# Extract information
single_vehicle = []
all_vehicles = []
for data_line in data:
	data_line = data_line.replace(",","")
	data_line = data_line.replace("[","")
	data_line = data_line.replace("]"," ")
	if data_line[0] != 'n':
		data_line = [int(x) for x in data_line.split()]
		cur_corner = []
		cur_all_corners = []
		for i in range(len(data_line)):
			cur_corner.append(data_line[i])
			if i%2:
				cur_all_corners.append(tuple(cur_corner))
				cur_corner = []
		single_vehicle.append(cur_all_corners)
	else:
		all_vehicles.append(single_vehicle)
		single_vehicle = []

# Filter out those with 2 or less captured frames
all_vehicles = filter(lambda x: len(x) > 2, all_vehicles)

lines_v1 = []

for vehicle in all_vehicles:
	# Extract single line from 4 corners
	for i in range(4):
		corners = [vehicle[x][i] for x in range(len(vehicle))]

		# Linear regression
		x_array = [corner[0] for corner in corners]
		y_array = [corner[1] for corner in corners]

		b, a, r = stats.linregress(x_array, y_array)[:3]
		if r**2 >= 0.995:
			lines_v1.append((a, b))

lines_sample = lines_v1[:1000]

# Linear regression analysis
a_array = [line[0] for line in lines_sample]
b_array = [line[1] for line in lines_sample]

x_v1, y_v1, r = stats.linregress(b_array, a_array)[:3]

print x_v1, y_v1, r**2

# Diamond space analysis
# Original parameters
d = 1
D = 1
width = 500
height = 500
multiplier = 800.0

def sgn(x):
	if x > 0:
		return 1
	return -1

def homogenize_line(line):
	a, b = line
	global multiplier
	# y = bx + a
	# bx - y + a = 0
	return [b, -1, a/multiplier]

def poly_d(h_line):
	a, b, c = h_line
	global d, D

	alpha = sgn(a*b)
	beta = sgn(b*c)
	gamma = sgn(c*a)
	
	p1 = (d*alpha*a/(c + gamma*a), D*-alpha*c/(c + gamma*a))
	p2 = (d*b/(c + beta*b), 0)
	p3 = (0, D*b/(a + alpha*b))
	p4 = (-d*alpha*a/(c + gamma*a), D*alpha*c/(c + gamma*a))

	return [p1, p2, p3, p4]

def cv_original2diamond(point):
	x, y, w = point
	return (-d*D*w, -d*x, sgn(x)*sgn(y)*x + y + sgn(y)*d*w)

def cv_diamond2original(point):
	x, y, w = point
	X = -y/d
	W = -x/(d*D)

	Ys = [w - X - d*W, w - X + d*W, w + X - d*W, w + X + d*W]

 	return [(X, Y, W) for Y in Ys]

# Generate blank table
table = []
for i in range(width):
	row = []
	for j in range(height):
		row.append(0)
	table.append(row)

def add_point(x, y, size):

 	global table
 	center = int(size/2)

 	# Generate dilation matrix
 	dilation_matrix = []
 	base = 100
 	factor = 0.6
 	for i in range(size):
 		row = []
 		for j in range(size):
 			dist = abs(i - center) + abs(j - center)
 			row.append(base*factor**dist)
 		dilation_matrix.append(row)

 	# Generate candidates for dilation
 	cands_sq = []
 	for x_sq in range(x - size/2, x + size/2 + 1):
 		for y_sq in range(y - size/2, y + size/2 + 1):
 			if x_sq >= 0 and y_sq >= 0 and x_sq < width and y_sq <= height:
 				cands_sq.append((x_sq, y_sq))

 	# Add points
 	for candidate in cands_sq:
 		x_sq, y_sq = candidate
 		dil_x = x_sq - x + size/2
 		dil_y = y_sq - y + size/2
 		table[x_sq][y_sq] += dilation_matrix[dil_x][dil_y]
 		#print x_sq, y_sq, dil_x, dil_y

def display_table():
	global table
	for row in table:
		print row

def getline(p1, p2):
    x1, y1 = p1
    x2, y2 = p2

    if x1 != x2:
    	b = (y2 - y1)/(x2 - x1)
    	a = y1 - b*x1
    
    return (a, b)

d_lines = [poly_d(homogenize_line(line)) for line in lines_sample]

for endpoints in d_lines:
	low_endpoints = []
	for endpoint in endpoints:
		low_endpoint = [(endpoint[0] + 1)*width/2, (endpoint[1] + 1)*height/2]
		low_endpoints.append(low_endpoint)

	for i in range(len(low_endpoints) - 1):
		head = (low_endpoints[i][0], low_endpoints[i][1])
		tail = (low_endpoints[i + 1][0], low_endpoints[i + 1][1])
		line_ab = getline(head, tail)
		a, b = line_ab

		x_left = min(head[0], tail[0])
		x_right = max(head[0], tail[0])

		for x in range(int(x_left), int(x_right)):
			y = int(b*x + a)
			add_point(x, y, 5)

max_val = 0
rem_x = 0
rem_y = 0
for x in range(width):
	for y in range(height):
		if table[x][y] > max_val:
			rem_x = x
			rem_y = y
			max_val = table[x][y]

scaled_x = 1.0*rem_x/(width/2) - 1
scaled_y = 1.0*rem_y/(height/2) - 1
scaled = (scaled_x, scaled_y, 1)

candidates = cv_diamond2original(scaled)
for candidate in candidates:
	print multiplier*candidate[0]/candidate[2], multiplier*candidate[1]/candidate[2]