#include <cassert>
#include <iostream>
#include <QDir>
#include "inputManager.h"

inputManager::inputManager(QString& imagePath)
    : input_path_(imagePath),
      cur_image_(0)
{
    QDir dir(input_path_.toAscii().data(),"*.jpg");
    image_list_ = dir.entryList();
}

inputManager::~inputManager()
{
}

bool inputManager::getCurrentImage(QImage& image)
{
    bool retValue = false;
    if (image_list_.size() > 0 && cur_image_ < image_list_.size())
    {
        std::cout<<"image:"<<QString(input_path_ + image_list_[cur_image_]).toAscii().data()<<std::endl;
        image.load(input_path_ + image_list_[cur_image_]);
        scale(image);
        retValue =  true;
    }
    return retValue;
}

bool inputManager::getCurrentImagePath(QString& path)
{
    bool retValue = false;
    if (image_list_.size() > 0 && cur_image_ < image_list_.size())
    {
        path = QString(input_path_ + image_list_[cur_image_]);
        retValue =  true;
    }
    return retValue;
}

bool inputManager::getNextImage(QImage& image)
{
    bool retValue = false;
    //std::cout<<"Cur: "<<count<<" of : "<<image_list_.size()<<std::endl; 
    if (image_list_.size() > 0 && cur_image_ < image_list_.size())
    {
        image.load(input_path_ + image_list_[cur_image_]);
        scale(image);
        retValue =  true;
        cur_image_++;
    }
    return retValue;
}

bool inputManager::getNextImagePath(QString& path)
{
    bool retValue = false;
    int next_image = cur_image_ + 1;
    if (image_list_.size() > 0 && next_image < image_list_.size())
    {
        path = QString(input_path_ + image_list_[next_image]);
        retValue =  true;
    }
    return retValue;
}

bool inputManager::next()
{
    bool retValue = false;

    if (image_list_.size() > 0 && cur_image_ < image_list_.size())
    {
        retValue =  true;
        cur_image_++;
    }
    return retValue;
}

void inputManager::scale(QImage& image)
{
  //do scaling with given height
  assert(FRAME_WIDTH > 0);
  image = image.scaledToWidth(FRAME_WIDTH);
}
