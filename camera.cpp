#include "camera.h"
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

RNG rng(12345);

Camera::Camera()
{
	m_fps = 30;
}

bool Camera::open(std::string filename)
{
	m_fileName = filename;
	
	// Convert filename to number if you want to
	// open webcam stream
	std::istringstream iss(filename.c_str());
	int devid;
	bool isOpen;
	if(!(iss >> devid))
	{
		isOpen = m_cap.open(filename.c_str());
	}
	else 
	{
		isOpen = m_cap.open(devid);
	}
	
	if(!isOpen)
	{
		std::cerr << "Unable to open video file." << std::endl;
		return false;
	}
	
	// set framerate, if unable to read framerate, set it to 30
	m_fps = m_cap.get(CV_CAP_PROP_FPS);
	if(m_fps == 0)
		m_fps = 30;
}


void Camera::play()
{
	// Create main window
	bool isReading = true;
	// Compute time to wait to obtain wanted framerate
	int timeToWait = 1000/m_fps;
	Mat Img;

	Mat dst, m_frame_copy; 
	Mat edges;

	int compteur_de_frames = 0;
	
	Mat img_de_reference;

	bool premiere_frame_aquise = 0;

	Mat detect_voies_isole,img_de_reference_grey,gray_frame,voitures_only ;

	// seance du vendredi
	vector<vector<Point> > contours;

	int nb_voitures = 0;

	Mat edges_first,frame_HSV;

		
	while(isReading)
	{
		// Get frame from stream
		isReading = m_cap.read(m_frame);
		compteur_de_frames++;

		
		Canny(m_frame, edges_first, 50, 200);


		// si la lecture s'est bien passée
		if(isReading)
		{
			// On recupere que la premiere frame
			if (compteur_de_frames ==9){
				premiere_frame_aquise = 1;
				
				
				
				// on recupere la premiere frame
		 		img_de_reference = m_frame.clone();
				

				//HSVisé + en gris
				cvtColor(img_de_reference, frame_HSV, COLOR_BGR2HSV);
				cvtColor(img_de_reference, img_de_reference_grey, COLOR_BGR2GRAY);

				//On filtre le HSV via InRange
				inRange(frame_HSV,Scalar(25, 50, 70), Scalar(102 ,255, 255),frame_HSV);
				
				// On en sort les lignes de contour
				Canny(frame_HSV, edges_first, 50, 200);

				

				
				// -----HOUGH-------- creation d'une frame avec que les lignes des voies : attention au format similaire pour soustraire après !

				Mat detect_voies(img_de_reference.rows,img_de_reference.cols,CV_8UC1,Scalar(0,0,0));

				vector<Vec2f> lines; 
    			HoughLines(edges_first, lines, 1, CV_PI/180, 75, 0, 0 ); // runs the actual detection
    			for( size_t i = 0; i < lines.size(); i++ ){
		 			float rho = lines[i][0], theta = lines[i][1];
		 			Point pt1, pt2;
		 			double a = cos(theta), b = sin(theta);
		 			double x0 = a * rho, y0 = b * rho;
		 			pt1.x = cvRound(x0 + 1000 * (-b));
		 			pt1.y = cvRound(y0 + 1000 * (a));
		 			pt2.x = cvRound(x0 - 1000 * (-b));
		 			pt2.y = cvRound(y0 - 1000 * (a));

		 			line(detect_voies, pt1, pt2, Scalar(255, 255, 255), 3, 2);
					line(img_de_reference, pt1, pt2, Scalar(255, 0, 0), 3, 2);
					imshow("Ou detecte t-on les voies ?", img_de_reference);}

				detect_voies_isole = detect_voies.clone();
				
				threshold(detect_voies_isole, detect_voies_isole, 0, 255, THRESH_BINARY ); //binarise
				
			}



					
			Canny(m_frame, edges, 50, 200);

			

			threshold(edges, edges, 0, 255, THRESH_BINARY ); // on met les edges detecté en seuil binaire

			// si on a fait l'aquisition de la premmiere image
			if (premiere_frame_aquise == 1){

				cvtColor(m_frame, gray_frame, COLOR_BGR2GRAY);
				voitures_only = img_de_reference_grey-gray_frame;
				threshold(voitures_only, voitures_only, 100, 255, THRESH_BINARY ); //binarise pour pouvoir mettre au meme format

				// on fait une fermeture pour avoir des voitures pleines
				morphologyEx(voitures_only,voitures_only,MORPH_CLOSE,getStructuringElement(MORPH_ELLIPSE,Size(25,30)));
				Mat res ; // res est une image binaire
				res = (detect_voies_isole-edges+voitures_only);
				imshow("voitures + lignes voies",res);
			


				// compter les voitures 



				// On trace une ligne peage :
				Rect rectangle_peage(Point(0,120), Point(800,123));

				
				rectangle(m_frame,rectangle_peage,Scalar(255,255,0));

				findContours( voitures_only, contours, RETR_TREE, CHAIN_APPROX_SIMPLE );
				vector<vector<Point> > contours_poly( contours.size() );
    			vector<Rect> boundRect( contours.size() );

				for( size_t i = 0; i < contours.size(); i++ ){
        			approxPolyDP( contours[i], contours_poly[i], 3, true );
        			boundRect[i] = boundingRect( contours_poly[i] );
					}
    
    			for( size_t i = 0; i< contours.size(); i++ ) {
        			Scalar color = Scalar( 255, 255, 255 );
        			rectangle( voitures_only, boundRect[i].tl(), boundRect[i].br(), color, 2 );
					rectangle( m_frame, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 0), 2 );
					

				}

				

				for (size_t i = 0; i< boundRect.size(); i++){
					// on regarde le centre de chaque rectangle detecté
					Point center_rect((boundRect[i].x + boundRect[i].width/2), (boundRect[i].y + boundRect[i].height/2));
					if (rectangle_peage.contains(center_rect)){
						nb_voitures++;
					}


				}

				
					
				



				// comptage du nombre de voitures


				// on va maintenant reperer a chaque rectangle son cercle
				

				imshow("TP2 video",m_frame);
				//imshow( "Contours", voitures_only+detect_voies_isole );




			}
	

			

		}

			


		else	{
			std::cerr << "Unable to read device" << std::endl;
		}
		
		// If escape key is pressed, quit the program
		if(waitKey(timeToWait)%256 == 27)
		{
			std::cerr << "Stopped by user" << std::endl;
			isReading = false;
		}	
	}

	std::cout<<"nombre de voitures : "<< nb_voitures<< std::endl;	
}


bool Camera::close()
{
	// Close the stream
	m_cap.release();
	
	// Close all the windows
	destroyAllWindows();
	usleep(100000);
}
























