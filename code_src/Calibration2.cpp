/*
 * Calibration2.cpp
 *
 *  Created on: Jun 26, 2014
 *      Author: atabb
 *      Updated: May 25, 2018 to use Eigen instead of newmat.
 *      atabb
 */
#include "Calibration2.hpp"
#include "StringFunctions.hpp"


using namespace cv;

CaliObjectOpenCV2::CaliObjectOpenCV2(int i, int w, int h,  double s_w_i, double s_h_i){

	chess_w = w;
	chess_h = h;
	number_internal_images_written = 0;
	mean_reproj_error = 0;
	mm_height = s_h_i;
	mm_width = s_w_i;
	image_size =  cv::Size(0, 0);
	text_file = "";
	mean_ext_reproj_error = 0;

}


void CaliObjectOpenCV2::ReadImages(string internal_dir, bool flag){
	cv::Mat im;
	vector<string> im_names;
	string filename;
	string txt_ext = "txt";

	int count;


	im_names.clear();

	cout << "Current dir " << internal_dir << endl;
	ReadDirectory(internal_dir, im_names);


	for (int c = 0; c < int(im_names.size()); c++){
		filename = internal_dir + "/" + im_names[c];
		cout << "Image name " << im_names[c] << endl;
		if (filename.size() > 3 && filename.substr(filename.size() - 3, filename.size()) != txt_ext){
			cout << "Reading filename .... " << filename << endl;
			im = cv::imread(filename.c_str());
			if (flag == 0){
				internal_images.push_back(im);
			}	else {
				external_images.push_back(im);
			}
		}	else {
			text_file = filename;
			cout << "Set filename! " << text_file << endl;
		}
	}
}

bool CaliObjectOpenCV2::AccumulateCorners(bool draw_corners){

	cv::Mat im, gimage, result;
	string current_name;
	bool corner_found;
	bool some_found = false;
	string filename;
	char ch;
	int corner_count = chess_h*chess_w;

	vector<cv::Point2f> pointBuf;
	cv::Point2f first_point, last_point;
	vector<cv::Point2f> flipped_points(corner_count);

	cv::Size boardsize;
	boardsize.height = chess_h;
	boardsize.width = chess_w;

	number_internal_images_written = 0;
	cout << "Doing internal images now .... " << endl;
	for (int i = 0; i <  int(internal_images.size()); i++){
		cout << "Looking for corners " << i << endl;
		//for (int i = 0; i < int(images_to_process.size()); i++){
		// find corners

		im = internal_images[i];
		cv::cvtColor(im, gimage, CV_BGR2GRAY);

		// This is a difference in between OpenCV current version and past.
		//corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE | CV_CALIB_CB_FILTER_QUADS);
		/// Current version -- Opencv 3.4.0
		corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK + CALIB_CB_FILTER_QUADS);

		if (corner_found) {

			// need to flip the orientation, possibly ...
			first_point = pointBuf[0];
			last_point = pointBuf[chess_w*chess_h - 1];

			if (first_point.y < last_point.y){
				cout << "WRONG ORIENTATION! " << endl;
				for (int k=0; k<corner_count; k++) {

					flipped_points[k] = pointBuf[chess_w*chess_h - 1 - k];

				}

				pointBuf.swap(flipped_points);

			}

			some_found = true;
			//			// refine the corner positions
			cornerSubPix(gimage, pointBuf, Size(11, 11), Size(-1, -1),
					TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));


			// draw detected corner points in the current frame
			cv::drawChessboardCorners(internal_images[i], boardsize, pointBuf, true);

			all_points.push_back(pointBuf);

			cout << "Number of patterns " << all_points.size() << endl;
			number_internal_images_written++;
		}
	}



	cout << "Doing external images now .... " << endl;
	for (int i = 0; i <  int(external_images.size()); i++){
		cout << "Looking for corners " << i << endl;
		// find corners

		im = external_images[i];
		cv::cvtColor(im, gimage, CV_BGR2GRAY);

		cv::imwrite("gray.png", gimage);

		/// < OpenCV 3.4.0
		//corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE | CV_CALIB_CB_FILTER_QUADS);
		/// Current version -- Opencv 3.4.0
		corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK + CALIB_CB_FILTER_QUADS);

		if (!corner_found){
			cout << "Trying default option " << endl;
			corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf);
		}

		if (!corner_found){
			cout << "Trying  option one" << endl;
			//corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf, CV_CALIB_CB_NORMALIZE_IMAGE);
			corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CALIB_CB_NORMALIZE_IMAGE);
		}

		if (!corner_found){
			cout << "Trying  option two" << endl;
			//corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf, CV_CALIB_CB_FILTER_QUADS);
			corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CALIB_CB_FILTER_QUADS);
		}

		if (!corner_found){
			cout << "Trying  option three" << endl;
			//corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH);
			corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CALIB_CB_ADAPTIVE_THRESH);
		}

		if (corner_found) {

			// need to flip the orientation, possibly ...

			//corners[chess_w*chess_h];


			// TODO -- determine if the pattern is in landscape or portrait mode.  correct accordingly.


			first_point = pointBuf[0];
			last_point = pointBuf[chess_w*chess_h - 1];

			if (first_point.y < last_point.y){
				cout << "WRONG ORIENTATION! " << endl;

				for (int k=0; k<corner_count; k++) {

					flipped_points[k] = pointBuf[chess_w*chess_h - 1 - k];
				}

				pointBuf.swap(flipped_points);
			}

			some_found = true;
			// refine the corner positions
			cv::cornerSubPix( gimage, pointBuf, cv::Size(11,11),
					cv::Size(-1,-1), cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));

			if (draw_corners){
				// draw detected corner points in the current frame
				cv::drawChessboardCorners(external_images[i], boardsize, cv::Mat(pointBuf), true);
			}

			all_points.push_back(pointBuf);

			cout << "Number of patterns " << all_points.size() << endl;
		}	else {
			cout << "WARNING -- EXTERNAL NOT FOUND! " << endl;
			exit(1);
		}
	}


	if (external_images.size() > 0){
		image_size = external_images[0].size();
	}


	if (external_images.size() > 0){
		image_size = external_images[0].size();
	}

	return some_found;

}

bool CaliObjectOpenCV2::AccumulateCornersFlexibleExternal(bool draw_corners){

	//IplImage        *cimage = 0,		*gimage = 0, *result = 0;
	cv::Mat im, gimage, result;
	string current_name;
	bool corner_found;
	bool some_found = false;
	string filename;
	char ch;
	int corner_count = chess_h*chess_w;

	vector<cv::Point2f> pointBuf;
	cv::Point2f first_point, last_point;
	vector<cv::Point2f> flipped_points(corner_count);

	cv::Size boardsize;
	boardsize.height = chess_h;
	boardsize.width = chess_w;
	//bool

	number_internal_images_written = 0;
	cout << "Doing internal images now .... " << endl;
	for (int i = 0; i <  int(internal_images.size()); i++){
		cout << "Looking for corners " << i << endl;
		// find corners

		im = internal_images[i];
		cv::cvtColor(im, gimage, CV_BGR2GRAY);

		// < OpenCV 3.4
		//corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE | CV_CALIB_CB_FILTER_QUADS);
		// OpenCV version differences
		corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK + CALIB_CB_FILTER_QUADS);

		if (corner_found ) {

			// need to flip the orientation, possibly ...

			//corners[chess_w*chess_h];

			first_point = pointBuf[0];
			last_point = pointBuf[chess_w*chess_h - 1];

			if (first_point.y < last_point.y){
				//if (first_point.x > last_point.x){
				cout << "WRONG ORIENTATION! " << endl;


				for (int k=0; k<corner_count; k++) {

					flipped_points[k] = pointBuf[chess_w*chess_h - 1 - k];
				}

				pointBuf.swap(flipped_points);

			}

			some_found = true;
			// refine the corner positions
			cv::cornerSubPix( gimage, pointBuf, cv::Size(11,11),
					cv::Size(-1,-1), cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));


			if (draw_corners){
				// draw detected corner points in the current frame
				cv::drawChessboardCorners(internal_images[i], boardsize, pointBuf, true);
			}

			all_points.push_back(pointBuf);

			cout << "Number of patterns " << all_points.size() << endl;
			number_internal_images_written++;


		}
	}

	cout << "Doing external images now .... " << endl;
	// TODO
	for (int i = 0; i <  int(external_images.size()); i++){
		cout << "Looking for corners " << i << endl;

		// find corners

		im = external_images[i];
		cv::cvtColor(im, gimage, CV_BGR2GRAY);

		//corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE | CV_CALIB_CB_FILTER_QUADS);
		// OpenCV version differences
		corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK + CALIB_CB_FILTER_QUADS);

		// December 3, 2018 -- can uncomment if you like, but experience is that individually, trying these flags do not help with localizing the pattern.
//		if (!corner_found){
//			cout << "Trying default option " << endl;
//			corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf);
//		}
//
//		if (!corner_found){
//			cout << "Trying  option one" << endl;
//			//corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf, CV_CALIB_CB_NORMALIZE_IMAGE);
//			corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CALIB_CB_NORMALIZE_IMAGE);
//		}
//
//		if (!corner_found){
//			cout << "Trying  option two" << endl;
//			//corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf, CV_CALIB_CB_FILTER_QUADS);
//			corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CALIB_CB_FILTER_QUADS);
//		}
//
//		if (!corner_found){
//			cout << "Trying  option three" << endl;
//			//corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH);
//			corner_found = cv::findChessboardCorners(gimage, boardsize, pointBuf,  CALIB_CB_ADAPTIVE_THRESH);
//		}


		if (corner_found) {

			// need to flip the orientation, possibly ...
			first_point = pointBuf[0];
			last_point = pointBuf[chess_w*chess_h - 1];

			if (first_point.y < last_point.y){
				//if (first_point.x > last_point.x){
				cout << "WRONG ORIENTATION! " << endl;


				for (int k=0; k<corner_count; k++) {

					flipped_points[k] = pointBuf[chess_w*chess_h - 1 - k];

				}

				pointBuf.swap(flipped_points);

			}

			some_found = true;
			// refine the corner positions
			cv::cornerSubPix( gimage, pointBuf, cv::Size(11,11),
					cv::Size(-1,-1), cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));


			if (draw_corners){
				// draw detected corner points in the current frame
				cv::drawChessboardCorners(external_images[i], boardsize, cv::Mat(pointBuf), true);
			}

			all_points.push_back(pointBuf);


			cout << "Number of patterns " << all_points.size() << endl;
		}	else {
			all_points.push_back(vector<cv::Point2f>());
		}
	}


	if (external_images.size() > 0){
		image_size = external_images[0].size();
	}


	if (external_images.size() > 0){
		image_size = external_images[0].size();
	}

	return some_found;

}


struct CameraCaliData{
	vector< vector<cv::Point2f> >* image_Points;
	vector< vector<cv::Point3f> >* world_Points;
	vector<vector<double> > ps;
	vector<double>  vals;
};


void CaliObjectOpenCV2::Calibrate(std::ofstream& out, string write_directory){

	// need to make the points 3D

	vector< cv::Point3f> corners(chess_h*chess_w);
	string filename;


	cout << "Calibrating using " << all_points.size() << " patterns " << endl;

	int counter = 0;
	for( int i = 0; i < chess_h; ++i ){
		for( int j = 0; j < chess_w; ++j, counter++ ){
			corners[counter] = (cv::Point3f(float( j*mm_width ), float( i*mm_height ), 0));
		}
	}

	// b/c all of the positions are the same ....
	for (int i = 0; i < int(all_points.size()); i++){
		all_3d_corners.push_back(corners);
	}

	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);

	cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);
	//cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64F);

	vector<cv::Mat> rvecs, tvecs;

	double max_dim = image_size.width;
	max_dim < image_size.height ? max_dim = image_size.height : 0;

	double focal_length_px = max_dim*1.2;
	// TODO
	focal_length_px = 3000.0;

	cameraMatrix.at<double>(0, 0) = focal_length_px;
	cameraMatrix.at<double>(1, 1) = focal_length_px;

	cameraMatrix.at<double>(0, 2) = image_size.width/2;
	cameraMatrix.at<double>(1, 2) = image_size.height/2;

	cout << "initial camera matrix " << endl;

	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			cout << cameraMatrix.at<double>(i, j) << " ";
		}
		cout << endl;
	}

	cout << "Running calibration " << endl;
	cout << "Number of dist coefficients  = " << distCoeffs.rows << endl;



	// this one does not use the initial guess
	double rms = 0;

	if (text_file.size() == 0){
		//rms = cv::calibrateCamera(all_3d_corners, all_points, image_size, cameraMatrix, distCoeffs, rvecs, tvecs,
		//		CV_CALIB_RATIONAL_MODEL);
		// OpenCV versions
		rms = cv::calibrateCamera(all_3d_corners, all_points, image_size, cameraMatrix, distCoeffs, rvecs, tvecs,
				CALIB_RATIONAL_MODEL);


	}	else {
		ifstream in(text_file.c_str());
		string temp;
		in >> cameraMatrix.at<double>(0, 0);
		in >> temp;
		in >> cameraMatrix.at<double>(0, 2);
		in >> temp;
		in >> cameraMatrix.at<double>(1, 1);
		in >> cameraMatrix.at<double>(1, 2);
		in >> temp >> temp >> temp;
		in.close();
	}


	//double rms = cv::calibrateCamera(all_3d_corners, all_points, image_size, cameraMatrix, distCoeffs, rvecs, tvecs, CV_CALIB_USE_INTRINSIC_GUESS);

	cout << "rms " << rms << endl;
	cout << "camera matrix " << endl;


	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			cout << cameraMatrix.at<double>(i, j) << " ";
		}
		cout << endl;
	}

	cout << "Distortion " << endl;
	for (int i = 0; i < distCoeffs.rows; i++){
		cout << distCoeffs.at<double>(i, 0) << " ";
	}
	cout << endl;


	out << "Internal images used: " << number_internal_images_written << endl;
	out << "External images used: " << external_images.size() << endl;
	out << "rms " << rms << endl;
	out << "camera matrix " << endl;
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			out << cameraMatrix.at<double>(i, j) << " ";
		}
		out << endl;
	}

	out << "Distortion " << endl;
	for (int i = 0; i < distCoeffs.rows; i++){
		out << distCoeffs.at<double>(i, 0) << " ";
	}
	out << endl;

	//	vector<vector<double> > A;
	//		vector< double > k;
	//		vector< vector <double> > Rt;
	//		vector< vector< vector <double> > > Rts;

	cv::Mat rotMatrix = cv::Mat::eye(3, 3, CV_64F);
	vector< vector <double> > tempRt(3, vector<double>(4, 0));

	A.resize(3, vector<double>(3, 0));
	k.resize(8, 0);
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			A[i][j] = cameraMatrix.at<double>(i, j);
		}
	}

	for (int i = 0; i < distCoeffs.rows; i++){
		k[i] = distCoeffs.at<double>(i, 0);
	}


	double reproj_error = 0;
	vector<cv::Point2f> imagePoints2;
	double err;

	for (int m = number_internal_images_written; m < int(all_points.size()); m++){

		cv::projectPoints( cv::Mat(all_3d_corners[m]), rvecs[m], tvecs[m], cameraMatrix,  // project
				distCoeffs, imagePoints2);
		err = cv::norm(cv::Mat(all_points[m]), cv::Mat(imagePoints2), CV_L2);              // difference

		reproj_error        += err*err;                                             // su
	}

	out << endl << "Summed reproj error " << reproj_error << endl << endl;

	// we only want these for the external images ....
	for (int m = number_internal_images_written; m < int(all_points.size()); m++){
		cv::Rodrigues(rvecs[m], rotMatrix);

		for (int i = 0; i < 3; i++){
			for (int j = 0; j < 3; j++){
				tempRt[i][j] = rotMatrix.at<double>(i, j);
			}

			tempRt[i][3] = tvecs[m].at<double>(i);
		}

		Rts.push_back(tempRt);

		out << "Rt for cam " << m << endl;
		for (int i = 0; i < 3; i++){
			for (int j = 0; j < 4; j++){
				out << tempRt[i][j] << " ";
			}
			out << endl;
		}

		out << endl;
	}

	cout << "Finish with cali ... " << endl;

	cv::Mat view, rview, map1, map2;
	cv::Mat gray;
	cv::initUndistortRectifyMap(cameraMatrix, distCoeffs, cv::Mat(),
			cv::getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, image_size, 1, image_size, 0),
			image_size, CV_16SC2, map1, map2);

	for (int i = 0; i < int(external_images.size()); i++){
		cout << "Writing external " << i << endl;
		cv::remap(external_images[i], rview, map1, map2, cv::INTER_LINEAR);

		//cv::cvtColor(rview, gray, CV_BGR2GRAY);
		//cv::cvtColor(gray, rview, CV_GRAY2BGR);
		filename  = write_directory + "/ext" + ToString<int>(i) + ".png";
		cv::imwrite(filename.c_str(), rview);
	}
}

void CaliObjectOpenCV2::CalibrateFlexibleExternal(float initial_focal_px, int zero_tangent_dist, int zero_k3, std::ofstream& out, string write_directory){

	// need to make the points 3D
	string camera_ply_directory = write_directory + "/ply_cameras";
	string command = "mkdir " + camera_ply_directory;
	system(command.c_str());
	string camera_ply_file;


	vector< cv::Point3f> corners(chess_h*chess_w);
	string filename;


	cout << "Calibrating using " << all_points.size() << " patterns " << endl;

	int counter = 0;
	for( int i = 0; i < chess_h; ++i ){
		for( int j = 0; j < chess_w; ++j, counter++ ){
			corners[counter] = (cv::Point3f(float( j*mm_width ), float( i*mm_height ), 0));
		}
	}

	// b/c all of the positions are the same ....
	// create map ....
	vector<bool> pattern_detected;
	vector<int> mapping_from_limited_to_full;
	vector< vector< cv::Point2f> > all_points_wo_blanks;

	for (int i = 0; i < int(all_points.size()); i++){
		if (all_points[i].size() > 0){
			mapping_from_limited_to_full.push_back(i);
			all_points_wo_blanks.push_back(all_points[i]);
			all_3d_corners.push_back(corners);
			pattern_detected.push_back(true);
		}	else {
			pattern_detected.push_back(false);
		}

	}

	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);

	cv::Mat distCoeffs = cv::Mat::zeros(5, 1, CV_64F);
	//cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64F);

	vector<cv::Mat> rvecs, tvecs;

	double max_dim = image_size.width;
	max_dim < image_size.height ? max_dim = image_size.height : 0;

	double focal_length_px = max_dim*1.2;

	if (initial_focal_px > 0){

		focal_length_px = initial_focal_px;
	}

	cameraMatrix.at<double>(0, 0) = focal_length_px;
	cameraMatrix.at<double>(1, 1) = focal_length_px;

	cameraMatrix.at<double>(0, 2) = image_size.width/2;
	cameraMatrix.at<double>(1, 2) = image_size.height/2;

	cout << "initial camera matrix " << endl;

	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			cout << cameraMatrix.at<double>(i, j) << " ";
		}
		cout << endl;
	}

	cout << "Running calibration " << endl;
	cout << "Number of dist coefficients  = " << distCoeffs.rows << endl;


	double rms = 0;
	char ch;


	if (text_file.size() == 0){

		int flags = cv::CALIB_USE_INTRINSIC_GUESS;

		if (zero_tangent_dist && !zero_k3){
			flags = cv::CALIB_USE_INTRINSIC_GUESS | cv::CALIB_ZERO_TANGENT_DIST ;
		}

		if (zero_tangent_dist && zero_k3){
			flags = cv::CALIB_USE_INTRINSIC_GUESS | cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_K3;
		}

		if (!zero_tangent_dist && zero_k3){
			flags = cv::CALIB_USE_INTRINSIC_GUESS | cv::CALIB_FIX_K3;
		}

		cout << "Calibration flags. " << flags << endl;


		rms = cv::calibrateCamera(all_3d_corners, all_points_wo_blanks, image_size, cameraMatrix, distCoeffs, rvecs, tvecs,
				flags);

	}	else {
		ifstream in(text_file.c_str());
		string temp;
		in >> cameraMatrix.at<double>(0, 0);
		in >> temp;
		in >> cameraMatrix.at<double>(0, 2);
		in >> temp;
		in >> cameraMatrix.at<double>(1, 1);
		in >> cameraMatrix.at<double>(1, 2);
		in >> temp >> temp >> temp;
		in.close();

		cout << "initial camera matrix " << endl;

		for (int i = 0; i < 3; i++){
			for (int j = 0; j < 3; j++){
				cout << cameraMatrix.at<double>(i, j) << " ";
			}
			cout << endl;
		}

		//	rms = cv::calibrateCamera(all_3d_corners, all_points_wo_blanks, image_size, cameraMatrix, distCoeffs, rvecs, tvecs,
		//					CV_CALIB_USE_INTRINSIC_GUESS);
		// OpenCV versions
		rms = cv::calibrateCamera(all_3d_corners, all_points_wo_blanks, image_size, cameraMatrix, distCoeffs, rvecs, tvecs,
				CALIB_USE_INTRINSIC_GUESS);
	}


	cout << "rms " << rms << endl;
	cout << "camera matrix " << endl;

	Matrix3d internal;


	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			cout << cameraMatrix.at<double>(i, j) << " ";
			internal(i, j) = cameraMatrix.at<double>(i, j);
		}
		cout << endl;
	}

	cout << "Distortion " << endl;
	for (int i = 0; i < distCoeffs.rows; i++){
		cout << distCoeffs.at<double>(i, 0) << " ";
	}
	cout << endl;


	out << "Internal images used: " << number_internal_images_written << endl;
	out << "External images used: " << external_images.size() << endl;
	out << "rms " << rms << endl;
	out << "camera matrix " << endl;
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			out << cameraMatrix.at<double>(i, j) << " ";
		}
		out << endl;
	}

	out << "Distortion " << endl;
	for (int i = 0; i < distCoeffs.rows; i++){
		out << distCoeffs.at<double>(i, 0) << " ";
	}
	out << endl;

	cv::Mat rotMatrix = cv::Mat::eye(3, 3, CV_64F);
	vector< vector <double> > tempRt(3, vector<double>(4, 0));

	A.resize(3, vector<double>(3, 0));
	k.resize(8, 0);
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			A[i][j] = cameraMatrix.at<double>(i, j);
		}
	}

	for (int i = 0; i < distCoeffs.rows; i++){
		k[i] = distCoeffs.at<double>(i, 0);
	}
	cout << "Line 13378 " << endl;
	//char ch; cin >> ch;

	double reproj_error = 0;
	vector<cv::Point2f> imagePoints2;
	double err;

	for (int m = number_internal_images_written; m < int(all_points_wo_blanks.size()); m++){

		cv::projectPoints( cv::Mat(all_3d_corners[m]), rvecs[m], tvecs[m], cameraMatrix,  // project
				distCoeffs, imagePoints2);
		err = cv::norm(cv::Mat(all_points_wo_blanks[m]), cv::Mat(imagePoints2), CV_L2);              // difference
		reproj_error        += err*err;


		// su
	}

	out << endl << "Summed reproj error " << reproj_error << endl << endl;

	//Matrix temp;
	for (int m = number_internal_images_written; m < int(all_points.size()); m++){
		Rts.push_back(vector<vector <double> >());
	}

	int actual_number;

	MatrixXd RtEigen(3, 4);

	// we only want these for the external images ....
	for (int m = number_internal_images_written; m < int(all_points_wo_blanks.size()); m++){


		cv::Rodrigues(rvecs[m], rotMatrix);
		//cout << "after conversion " << endl; cin >> ch;

		for (int i = 0; i < 3; i++){
			for (int j = 0; j < 3; j++){
				tempRt[i][j] = rotMatrix.at<double>(i, j);
				RtEigen(i, j) = rotMatrix.at<double>(i, j);
			}

			tempRt[i][3] = tvecs[m].at<double>(i);
			RtEigen(i, 3) = tvecs[m].at<double>(i);

		}

		actual_number = mapping_from_limited_to_full.at(m) - number_internal_images_written;

		camera_ply_file = camera_ply_directory + "/camera" + ToString<int>(actual_number) + ".ply";

		create_camera(internal, RtEigen, 200, 0, 50, image_size.height, image_size.width, camera_ply_file, 200.0);


		Rts[mapping_from_limited_to_full.at(m) - number_internal_images_written] = tempRt;
		//cout << "RTs " << endl; cin >> ch;

		out << "Rt for cam " << mapping_from_limited_to_full[m] - number_internal_images_written << endl;
		for (int i = 0; i < 3; i++){
			for (int j = 0; j < 4; j++){
				out << tempRt[i][j] << " ";
			}
			out << endl;
		}

		out << endl;
	}

	cout << "Finish with cali ... " << endl;
	//cin >> ch;

	cv::Mat view, rview, map1, map2;
	cv::Mat gray;
	cv::initUndistortRectifyMap(cameraMatrix, distCoeffs, cv::Mat(),
			cv::getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, image_size, 1, image_size, 0),
			image_size, CV_16SC2, map1, map2);

	for (int i = 0; i < int(external_images.size()); i++){
		cout << "Writing external " << i << endl;
		cv::remap(external_images[i], rview, map1, map2, cv::INTER_LINEAR);

		//cv::cvtColor(rview, gray, CV_BGR2GRAY);
		//cv::cvtColor(gray, rview, CV_GRAY2BGR);
		filename  = write_directory + "/ext" + ToString<int>(i) + ".png";
		cv::imwrite(filename.c_str(), rview);
	}
}



void camera(Matrix3d& Kinv, float max_u, float max_v, float mag, vector< Vector3d >& vertex_coordinates ) {

	Vector3d a;
	Vector3d b;
	Vector3d c;
	Vector3d d;

	Vector3d x;
	x << 0, 0, 1;

	a = mag*Kinv*x;

	x << max_u, 0, 1;
	b = mag*Kinv*x;

	x << max_u, max_v, 1;
	c = mag*Kinv*x;

	x << 0, max_v, 1;
	d = mag*Kinv*x;

	vertex_coordinates.push_back(a);
	vertex_coordinates.push_back(d);
	vertex_coordinates.push_back(c);
	vertex_coordinates.push_back(b);

}

void create_camera4d(Matrix3d& internal, Matrix4d& external, int r, int g, int b, int rows, int cols,
		string ply_file, double scale){

	MatrixXd rt(3, 4);

	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 4; j++){
			rt(i, j) = external(i, j);
		}
	}

	create_camera(internal, rt, r, g, b, rows, cols, ply_file, scale);


}


int create_camera(Matrix3d& internal, MatrixXd& external, int r, int g, int b, int rows, int cols,
		string ply_file, double scale){

	vector< Vector3d > vertex_coordinates;
	vector< Vector3d > vertex_normals;
	vector< vector<int> > face_indices;
	vector< vector<int> > edge_indices;
	vector<Vector3d> colors;
	Vector3d C;
	Vector3d t;
	Matrix3d R;

	for (int r0 = 0; r0 < 3; r0++){
		for (int c = 0; c < 3; c++){
			R(r0, c) = external(r0, c);
		}

		t(r0) = external(r0, 3);
	}


	C = -R.inverse()*t;

	Matrix3d Kinv = internal.inverse();


	Matrix3d Rinv = R.transpose();

	camera(Kinv, cols, rows, scale, vertex_coordinates);


	Vector3d tempV;


	for (int i = 0; i < 4; i++){
		tempV = vertex_coordinates[i];
		vertex_coordinates[i] = Rinv*(vertex_coordinates[i] - t);

	}

	Vector3d diff_vector;

	vertex_coordinates.push_back(C);


	Vector3d cp;
	int vertex_number = 0;

	// front face.
	vector<int> face;
	face.push_back(vertex_number);
	face.push_back(vertex_number + 3);
	face.push_back(vertex_number + 1);

	face_indices.push_back(face);


	face.clear();

	face.push_back(vertex_number + 2);
	face.push_back(vertex_number + 1);
	face.push_back(vertex_number + 3);

	face_indices.push_back(face);


	face.clear();
	// a side.

	face.push_back(vertex_number);
	face.push_back(vertex_number + 4);
	face.push_back(vertex_number + 3);


	face_indices.push_back(face);

	face.clear();

	face.push_back(vertex_number);
	face.push_back(vertex_number + 1);
	face.push_back(vertex_number + 4);


	face_indices.push_back(face);

	face.clear();

	face.push_back(vertex_number + 1);
	face.push_back(vertex_number + 2);
	face.push_back(vertex_number + 4);


	face_indices.push_back(face);

	face.clear();

	face.push_back(vertex_number + 2);
	face.push_back(vertex_number + 3);
	face.push_back(vertex_number + 4);


	face_indices.push_back(face);

	face.clear();


	vertex_number += 5;


	std::ofstream out;
	out.open(ply_file.c_str());


	out << "ply" << endl;
	out << "format ascii 1.0" << endl;
	out << "element vertex " << vertex_coordinates.size() << endl;
	out << "property float x" << endl;
	out << "property float y" << endl;
	out << "property float z" << endl;
	//out << "property float nx" << endl;
	//out << "property float ny" << endl;
	//out << "property float nz" << endl;
	out << "property uchar red" << endl;
	out << "property uchar green" << endl;
	out << "property uchar blue" << endl;
	out << "property uchar alpha" << endl;
	out << "element face " << face_indices.size() << endl;
	out << "property list uchar int vertex_indices"<< endl;

	out << "end_header" << endl;

	unsigned int zero = 0;
	for (int i = 0, nc = vertex_coordinates.size(); i < nc; i++){
		out << vertex_coordinates[i](0) << " " << vertex_coordinates[i](1) << " " << vertex_coordinates[i](2) << " ";

		if (i == 0){
			out << " 255 255 255 255" << endl;
		}	else {
			out << r << " " << g << " " << b << " 255 " << endl;
		}


	}

	for (int i = 0; i < int(face_indices.size()); i++){

		out << face_indices[i].size() << " ";


		for (int j = 0; j < int(face_indices[i].size()); j++){
			out << face_indices[i].at(j) << " ";
		}


		out << endl;

	}


	out.close();


	return 0;
}
