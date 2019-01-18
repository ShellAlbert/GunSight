#include "CSK_Tracker.h"

using namespace std;

const double CSK_Tracker::padding =0;//1;
const double CSK_Tracker::output_sigma_factor = 1.0/16;
const double CSK_Tracker::sigma = 0.2;
const double CSK_Tracker::lambda = 0.01;
const double CSK_Tracker::interp_factor = 0.075;

CSK_Tracker::CSK_Tracker()
{
	
}

CSK_Tracker::~CSK_Tracker()
{
	
}

void CSK_Tracker::hann2d(Mat& m)
{
	Mat a(m.rows,1,CV_32FC1);
	Mat b(m.cols,1,CV_32FC1);
	for (int i = 0; i < m.rows; i++)
	{
		float t = 0.5 * (1 - cos(2*CV_PI*i/(m.rows - 1)));
		a.at<float>(i,0) = t;
	}
	for (int i = 0; i < m.cols; i++)
	{
		float t = 0.5 * (1 - cos(2*CV_PI*i/(m.cols - 1)));
		b.at<float>(i,0) = t;
	}
	m = a * b.t();
}

Mat CSK_Tracker::dense_guess_kernel(double sigma, Mat x, Mat y)
{
	//xf = fft2(x)
	Mat xf = fft2d(x);
	vector<Mat> xf_ri(xf.channels());
	cv::split(xf,xf_ri);
	
	//xx = x(:)' * x(:);
	double xx = 0;
	Scalar sum_x = sum(x.mul(x));
	for (int i = 0; i < sum_x.cols; i++)
	{
		xx += sum_x[i];
	}

	//yf = fft2(y)
	Mat yf = fft2d(y);
	vector<Mat> yf_ri(yf.channels());
	cv::split(yf,yf_ri);

	//yy = y(:)' * y(:);
	double yy = 0;
	Scalar sum_y = sum(y.mul(y));
	for (int i = 0; i < sum_y.cols; i++)
	{
		yy += sum_y[i];
	}

	//xyf = xf. * conj(yf) 
	Mat xyf = c_mul(xf,conj(yf));

	//xy = real(circshift(ifft2(xyf), floor(size(x)/2)));
	idft(xyf,xyf);
	xyf = xyf/(xyf.rows*xyf.cols);

	vector<Mat> xy_ri(xyf.channels());
	cv::split(xyf,xy_ri);
	Mat xy = xy_ri[0];

	int cx = xy.cols/2;
	int cy = xy.rows/2;
	Mat q0(xy, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant 
	Mat q1(xy, Rect(cx, 0, cx, cy));  // Top-Right
	Mat q2(xy, Rect(0, cy, cx, cy));  // Bottom-Left
	Mat q3(xy, Rect(cx, cy, cx, cy)); // Bottom-Right
	Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);

	int numel_x = x.rows*x.cols;
	Mat k;
	exp((-1/pow(sigma,2))*max((xx+yy-2*xy)/numel_x,0),k);

	return k;
}

Mat CSK_Tracker::dense_guess_kernel(double sigma, Mat x)
{
	//xf = fft2(x)
	Mat xf = fft2d(x);
	vector<Mat> xf_ri(xf.channels());
	cv::split(xf,xf_ri);

	//xx = x(:)' * x(:);
	double xx = 0;
	Scalar sum_x = sum(x.mul(x));
	for (int i = 0; i < sum_x.cols; i++)
	{
		xx += sum_x[i];
	}

	//yf = xf
	//yy = xx
	Mat yf;
	xf.copyTo(yf);
	double yy = xx;
	vector<Mat> yf_ri(yf.channels());
	cv::split(yf,yf_ri);

	//xyf = xf. * conj(yf) 
	Mat xyf = c_mul(xf,conj(yf));

	//xy = real(circshift(ifft2(xyf), floor(size(x)/2)));
	idft(xyf,xyf);
	xyf = xyf/(xyf.rows*xyf.cols);

	vector<Mat> xy_ri(xyf.channels());
	cv::split(xyf,xy_ri);
	Mat xy = xy_ri[0];
	//print_mat(xy,"xyf.txt");

	int cx = xy.cols/2;
	int cy = xy.rows/2;
	Mat q0(xy, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant 
	Mat q1(xy, Rect(cx, 0, cx, cy));  // Top-Right
	Mat q2(xy, Rect(0, cy, cx, cy));  // Bottom-Left
	Mat q3(xy, Rect(cx, cy, cx, cy)); // Bottom-Right
	Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);
	q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);

	int numel_x = x.rows*x.cols;
	Mat k;
	exp((-1/pow(sigma,2))*max((xx+yy-2*xy)/numel_x,0),k);

	return k;
}

Mat CSK_Tracker::get_subwindow(Mat im, Point pos, int* sz, Mat cos_window)
{
	//xs = floor(pos(2)) + (1:sz(2)) - floor(sz(2)/2);
	//ys = floor(pos(1)) + (1:sz(1)) - floor(sz(1)/2);
	vector<int> xs(sz[1]);
	vector<int> ys(sz[0]);
	for (int i = 0; i < sz[1]; i++)
	{
		xs[i] = floor(pos.x) + i - floor(sz[1]/2);
		xs[i] = max(min(xs[i],im.cols - 1),0);
	}
	for (int i = 0; i < sz[0]; i++)
	{
		ys[i] = floor(pos.y) + i - floor(sz[0]/2);
		ys[i] = max(min(ys[i],im.cols - 1),0);
	}

	Mat out(sz[0],sz[1],CV_32FC1);
	for (int i = 0; i < sz[0]; i++)
	{
		for (int j = 0; j < sz[1]; j++)
		{
			out.at<float>(i,j) = float(im.at<uchar>(ys[i],xs[j]))/255 - 0.5;
		}
	}
	out = cos_window.mul(out);
	return out;
}

void CSK_Tracker::tracke_one(ifstream &infile, Point pos_first, int frame_count, Size target_sz)
{

	//window size, taking padding into account
	int sz[2] = {floor(target_sz.height * (1 + padding)),floor(target_sz.width * (1 + padding))};

	//desired output (gaussian shaped), bandwidth proportional to target size
	double output_sigma = sqrt(target_sz.height * target_sz.width) * output_sigma_factor;
	Mat rs(sz[0],sz[1],CV_32FC1);
	Mat cs(sz[0],sz[1],CV_32FC1);
	for (int i = 0; i < sz[0]; i++)
	{
		for (int j = 0; j < sz[1]; j++)
		{
			rs.at<float>(i,j) = i - sz[0]/2 +1;
			cs.at<float>(i,j) = j - sz[1]/2 +1;
		}
	}

	Mat y;
	exp((-0.5/pow(output_sigma,2))*(rs.mul(rs) + cs.mul(cs)),y);

	//yf = fft2(y)
	Mat yf;
	//IplImage *y_temp = &IplImage(y);
	yf = fft2d(y);
	vector<Mat> yf_ri(yf.channels());
	cv::split(yf,yf_ri);

	//store pre-computed cosine window
	Mat cos_window(sz[0],sz[1],CV_32FC1);
	hann2d(cos_window);

	Mat frame;
	Mat x;
	Mat k;
	Mat z;
	Mat alphaf;
	Mat new_alphaf;

	Point pos = pos_first;

	namedWindow("tracking");
	string file_name;

	for(int i = 0; i < frame_count; ++i){
		
		getline(infile,file_name);
		frame = imread(file_name);

		if(frame.empty())
		{
			cout << "fail to open frame" << i << endl;
			break;
		}

		if (frame.channels() > 1)
		{
            cvtColor(frame,frame,COLOR_BGR2GRAY);
		}
		//extract and pre-process subwindow

		x = get_subwindow(frame, pos, sz, cos_window);

		if (i > 0)
		{
			k = dense_guess_kernel(sigma,x,z);

			//kf = fft2(k)
			//IplImage* k_temp = &IplImage(k);
			Mat kf = fft2d(k);
			vector<Mat> kf_ri(kf.channels());
			cv::split(kf,kf_ri);

			//response = real(ifft2(alphaf .* fft2(k)));   %(Eq. 9)
			vector<Mat> response_ri(2);
			Mat response = c_mul(alphaf,kf);
			idft(response,response);
			response = response/(response.rows*response.cols);
			cv::split(response,response_ri);
			//print_mat(response_ri[0],"response.txt");

			//target location is at the maximum response
			int max_row, max_col;
			double max_response = 0;
			for (int j = 0; j < response_ri[0].rows; j++)
			{
				for (int k = 0; k < response_ri[0].cols; k++)
				{
					if (response_ri[0].at<float>(j,k) > max_response)
					{
						max_response = response_ri[0].at<float>(j,k);
						max_row = j;
						max_col = k;
					}
				}
			}
			pos = pos - Point(floor(sz[1]/2),floor(sz[0]/2)) + Point(max_col+1,max_row+1);
		}

		x = get_subwindow(frame,pos,sz,cos_window);
		k = dense_guess_kernel(sigma,x);
		//new_alphaf = yf ./ (fft2(k) + lambda);   %(Eq. 7)
		//IplImage *k_t = &IplImage(k);

		new_alphaf = c_div(yf,(fft2d(k) + lambda));
		vector<Mat> new_alphaf_ri(2);
		cv::split(new_alphaf,new_alphaf_ri);

		Mat new_z = x;

		if (i == 0)
		{
			alphaf = new_alphaf;
			z = x;
		}
		else
		{
			alphaf = (1 - interp_factor) * alphaf +interp_factor*new_alphaf;
			z = (1 - interp_factor) * z + interp_factor * new_z;
		}
		
		Mat frame_print;
		frame.copyTo(frame_print);
		rectangle(frame_print,Point(pos.x - target_sz.width/2,pos.y - target_sz.height/2),Point(pos.x + target_sz.width/2,pos.y + target_sz.height/2),CV_RGB(255,255,255),1);
        //circle(frame_print,Point(pos.x,pos.y),2,cvScalar(255,0,0));
		imshow("tracking",frame_print);
        //cvWaitKey(10);
	}

	return;
}

bool CSK_Tracker::tracker_init(Mat frame, Point pos, Size target_sz)
{
	//window size, taking padding into account
	int sz[2] = {floor(target_sz.height * (1 + padding)),floor(target_sz.width * (1 + padding))};
	
	//desired output (gaussian shaped), bandwidth proportional to target size
	double output_sigma = sqrt(target_sz.height * target_sz.width) * output_sigma_factor;
	
	Mat rs(sz[0],sz[1],CV_32FC1);
	Mat cs(sz[0],sz[1],CV_32FC1);
	
	for (int i = 0; i < sz[0]; i++)
	{
		for (int j = 0; j < sz[1]; j++)
		{
			rs.at<float>(i,j) = i - sz[0]/2 +1;
			cs.at<float>(i,j) = j - sz[1]/2 +1;
		}
	}

	Mat y;
	exp((-0.5/pow(output_sigma,2))*(rs.mul(rs) + cs.mul(cs)),y);
	
	//yf = fft2(y)
	Mat yf;
	//IplImage *y_temp = &IplImage(y);
	yf = fft2d(y);
	vector<Mat> yf_ri(yf.channels());
	cv::split(yf,yf_ri);

	//store pre-computed cosine window
	Mat cos_window(sz[0],sz[1],CV_32FC1);
	hann2d(cos_window);
	
	Mat x;
	Mat k;
	Mat z;
	Mat alphaf;
	Mat new_alphaf;
		
	if(frame.empty())
	{
		cout << "fail to open frame" << endl;
		return false;
	}
	
//	Mat frame_temp;
//	frame.copyTo(frame_temp);	

	if (frame.channels() > 1)
	{
        cvtColor(frame,frame,COLOR_BGR2GRAY);
	}	
	
	x = get_subwindow(frame, pos, sz, cos_window);
	k = dense_guess_kernel(sigma,x);			
	
	//new_alphaf = yf ./ (fft2(k) + lambda);   %(Eq. 7)
	new_alphaf = c_div(yf,(fft2d(k) + lambda));
	vector<Mat> new_alphaf_ri(2);
	cv::split(new_alphaf,new_alphaf_ri);	
		
	m_init_yf = yf;
	m_init_cos_window = cos_window;
	
	m_update_z = x;
	m_update_alphaf = new_alphaf;
	
	return true;
}

bool CSK_Tracker::tracker_update(Mat frame, Point &pos, Size target_sz)
{
	//window size, taking padding into account
	int sz[2] = {floor(target_sz.height * (1 + padding)),floor(target_sz.width * (1 + padding))};
	
	Mat yf = m_init_yf;
	Mat cos_window = m_init_cos_window;
	
	Mat z = m_update_z;
	Mat alphaf = m_update_alphaf;
	
	Mat x;
	Mat k;
	Mat new_alphaf;

	if(frame.empty())
	{
		cout << "fail to open frame" << endl;
		return false;
	}
//	Mat frame_temp;
//	frame.copyTo(frame_temp);	
	
	if (frame.channels() > 1)
	{
        cvtColor(frame,frame,COLOR_BGR2GRAY);
	}

	//extract and pre-process subwindow
	x = get_subwindow(frame, pos, sz, cos_window);
	k = dense_guess_kernel(sigma,x,z);

	//kf = fft2(k)
	Mat kf = fft2d(k);
	vector<Mat> kf_ri(kf.channels());
	cv::split(kf,kf_ri);

	//response = real(ifft2(alphaf .* fft2(k)));   %(Eq. 9)
	vector<Mat> response_ri(2);
	Mat response = c_mul(alphaf,kf);
	idft(response,response);
	response = response/(response.rows*response.cols);
	cv::split(response,response_ri);

	//%target location is at the maximum response
	int max_row, max_col;
	double max_response = 0;
	for (int j = 0; j < response_ri[0].rows; j++)
	{
		for (int k = 0; k < response_ri[0].cols; k++)
		{
			if (response_ri[0].at<float>(j,k) > max_response)
			{
				max_response = response_ri[0].at<float>(j,k);
				max_row = j;
				max_col = k;
			}
		}
	}
	pos = pos - Point(floor(sz[1]/2),floor(sz[0]/2)) + Point(max_col+1,max_row+1);
	if (pos.x < 0 || pos.x > frame.cols || pos.y < 0 || pos.y > frame.rows)
	{
		return false;
	}

	x = get_subwindow(frame, pos, sz, cos_window);
	k = dense_guess_kernel(sigma, x);
	
	//new_alphaf = yf ./ (fft2(k) + lambda);   %(Eq. 7)
	new_alphaf = c_div(yf,(fft2d(k) + lambda));
	vector<Mat> new_alphaf_ri(2);
	cv::split(new_alphaf,new_alphaf_ri);

	Mat new_z = x;

	m_update_alphaf = (1 - interp_factor) * alphaf +interp_factor * new_alphaf;
	m_update_z = (1 - interp_factor) * z + interp_factor * new_z;

	return true;
}

Mat CSK_Tracker::conj(Mat a)
{
	Mat b;
	a.copyTo(b);
	vector<Mat> b_ri(2);
	cv::split(b,b_ri);
	b_ri[1] = -b_ri[1];
	merge(b_ri,b);
	return b;
}

Mat CSK_Tracker::c_mul(Mat a, Mat b)
{
	if (!(a.channels() == 2 || b.channels() == 2))
	{
		cout << "c_mul error!" << endl;
	}
	vector<Mat> a_ri(2);
	vector<Mat> b_ri(2);
	cv::split(a,a_ri);
	cv::split(b,b_ri);
	vector<Mat> c_ri(2);
	c_ri[0] = a_ri[0].mul(b_ri[0]) - a_ri[1].mul(b_ri[1]);
	c_ri[1] = a_ri[0].mul(b_ri[1]) + a_ri[1].mul(b_ri[0]);
	Mat c;
	merge(c_ri,c);
	return c;
}
Mat CSK_Tracker::c_div(Mat a, Mat b)
{
	Mat c;
	c = c_mul(a,conj(b));
	vector<Mat> c_ri(2);
	cv::split(c,c_ri);
	vector<Mat> mag_b_ri(2);
	Mat mag_b = c_mul(b,conj(b));
	cv::split(mag_b,mag_b_ri);
	c_ri[0] = c_ri[0]/mag_b_ri[0];
	c_ri[1] = c_ri[1]/mag_b_ri[0];
	merge(c_ri,c);
	return c;
}

Mat CSK_Tracker::fft2d(Mat a)
{
	Mat padded_a;

	int m_a = a.rows;
	int n_a = a.cols;
	copyMakeBorder(a, padded_a, 0, m_a - a.rows, 0, n_a - a.cols, BORDER_CONSTANT, Scalar::all(0));
	Mat planes_a[] = {Mat_<float>(padded_a), Mat::zeros(padded_a.size(),CV_32F)};
	Mat af;
	merge(planes_a, 2, af);
	dft(af,af);
	return af;
}

void CSK_Tracker::print_mat(Mat a, string file_name)
{
	ofstream fout(file_name);
	int col = a.cols;
	int row = a.rows;
	for(int i = 0; i< row; i++)
	{
		for(int j = 0; j < col; j++)
		{
			fout << a.at<float>(i,j) << " ";
		}
		fout << '\n';
	}
	fout.close();
}

void CSK_Tracker::print_img(Mat a, string file_name)
{
	ofstream fout(file_name);
	int col = a.cols;
	int row = a.rows;
	for(int i = 0; i< row; i++)
	{
		for(int j = 0; j < col; j++)
		{
			fout << float(a.at<uchar>(i,j)) << " ";
		}
		fout << endl;
	}
	fout.close();
}
